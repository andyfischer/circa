
# ast.py
#
# Defines classes for an abstract syntax tree

import parse_errors, tokens
from Circa.core import (builtins, ca_string, ca_function, ca_type)
from Circa.core.term import Term
from Circa.common import (debug, errors)
from Circa.utils.spy_object import SpyObject
from token_definitions import *

class CompilationContext(object):
    def __init__(self, resultCU, compilationCU):
        self.resultCU = resultCU
        self.compilationCU = compilationCU

class Node(object):
    def createTerms(self, context):
        raise errors.PureVirtualMethodFail(self, 'createTerms')
    def renderSource(self):
        raise errors.PureVirtualMethodFail(self, 'renderSource')
    def getFirstToken(self):
        raise errors.PureVirtualMethodFail(self, 'getFirstToken')
    def __str__(self):
        return self.renderSource()

class StatementList(object):
    def __init__(self):
        self.statements = []
    def createTerms(self, context):
        for statement in self.statements:
            statement.createTerms(context)
    def getFirstToken(self):
        return self.statements[0].getFirstToken()
    def renderSource(self):
        return "".join([statement.renderSource() for statement in self.statements])

class IgnoredSyntax(Node):
    def __init__(self, token):
        self.token = token
    def createTerms(self, context):
        pass
    def renderSource(self):
        return self.token.text

class Infix(Node):
    def __init__(self, functionToken, left, right):
        assert isinstance(left, Node)
        assert isinstance(right, Node)

        self.token = functionToken
        self.left = left
        self.right = right

    def createTerms(self, context):

        # Evaluate as an assignment?
        if self.token.match == EQUALS:
            right_term = self.right.createTerms(context)
            if not isinstance(right_term, Term):
                raise parse_errors.ExpressionDidNotEvaluateToATerm(self.right.getFirstToken())
            context.resultCU.bindName(right_term, str(self.left))
            return right_term

        # Evaluate as feedback?
        if self.token.match == COLON_EQUALS:
            leftTerm = self.left.createTerms(context)
            rightTerm = self.right.createTerms(context)
            debug._assert(leftTerm is not None)
            debug._assert(rightTerm is not None)
            return context.compilationCU.createTerm(
                    builtins.FEEDBACK_FUNC, [leftTerm, rightTerm])

        # Evaluate as a right-arrow?
        # (Not supported yet)
        """
        if self.token.match == RIGHT_ARROW:
            left_inputs = self.left.createTerms(context)
            right_func = self.right.createTerms(context)

            return context.createTerm(right_func, inputs=[left_inputs])
        """

        # Normal function?
        # Try to find a defined operator
        normalFunction = getOperatorFunction(context.resultCU, self.token)
        if normalFunction is not None:

            newTerm = context.resultCU.createTerm(normalFunction,
                inputs=[self.left.createTerms(context),
                        self.right.createTerms(context)])
            newTerm.ast = self
            return newTerm

        # Evaluate as a function + assign?
        # Try to find an assign operator
        assignFunction = getAssignOperatorFunction(self.token.match)
        if assignFunction is not None:
            # create a term that's the result of the operation
            result_term = context.resultCU.createTerm(assignFunction,
               inputs=[self.left.createTerms(context), self.right.createTerms(context)])

            # bind the name to this result
            context.resultCU.bindName(result_term, str(self.left))
            return result_term

        debug.fail("Unable to evaluate token: " + self.token.text)

    def getFirstToken(self):
        return self.left.getFirstToken()

    def renderSource(self):
        return (self.left.renderSource() + ' ' + self.token.text + ' '
            + self.right.renderSource())

class Literal(Node):
    def __init__(self, token, hasQuestionMark=False):
        self.token = token
        self.hasQuestionMark = hasQuestionMark

        if token.match == FLOAT:
            self.value = float(token.text)
            self.circaType = builtins.FLOAT_TYPE
        elif token.match == INTEGER:
            self.value = int(token.text)
            self.circaType = builtins.INT_TYPE
        elif token.match == STRING:
            self.value = parseStringLiteral(token.text)
            self.circaType = builtins.STRING_TYPE
        else:
            raise parse_errors.InternalError("Couldn't recognize token: " + str(token))

    def createTerms(self, context):
        # Create a term
        if self.hasQuestionMark:
            newTerm = context.resultCU.createVariable(self.circaType)
        else:
            newTerm = context.resultCU.createConstant(self.circaType)

        newTerm.ast = self

        # Assign value
        # Future: do this in a more typesafe way
        newTerm.cachedValue = self.value

        return newTerm

    def getFirstToken(self):
        return self.token

    def renderSource(self):
        if (self.token.match == STRING):
            return "'" + self.value + "'"
        else:
            return str(self.value)

class Ident(Node):
    def __init__(self, token):
        self.token = token

    def createTerms(self, context):
        term = context.resultCU.getNamed(self.token.text)

        if not term:
            raise parse_errors.IdentifierNotFound(self.token)

        return context.resultCU.getNamed(self.token.text)

    def getFirstToken(self):
        return self.token

    def renderSource(self):
        return self.token.text

class Unary(Node):
    def __init__(self, functionToken, right):
        self.functionToken = functionToken
        self.right = right

    def createTerms(self, context):
        return context.resultCU.createTerm(builtins.MULT,
                   inputs = [context.resultCU.createConstant(-1),
                             self.right.createTerms(context)])

    def getFirstToken(self):
        return self.functionToken;

    def renderSource(self):
        return '-' + self.right.renderSource()

class FunctionCall(Node):
    def __init__(self, function_name, args):
        self.function_name = function_name
        self.args = args

    def createTerms(self, context):
        arg_terms = [term.createTerms(context) for term in self.args]
        func = context.resultCU.getNamed(self.function_name.text)

        if func is None:
            raise parse_errors.InternalError(self.function_name,
              "Function " + self.function_name.text + " not found.")

        # Check for Function
        if func.getType() is builtins.FUNCTION_TYPE:
            newTerm = context.resultCU.createTerm(func, inputs=arg_terms)
            newTerm.ast = self
            return newTerm

        # Check for Subroutine
        elif func.getType() is builtins.SUBROUTINE_TYPE:

            # Todo: special behavior for invoking subroutines
            return context.resultCU.createTerm(builtins.INVOKE_SUB_FUNC)

        # Temp: Use a Python dynamic type check to see if this is a function
        elif isinstance(func.pythonValue, ca_function._Function):
            return context.resultCU.createTerm(func, inputs=arg_terms)

        else:
            raise parse_errors.InternalError(self.function_name,
               "Term " + self.function_name.text + " is not a function.")

    def getFirstToken(self):
        return self.function_name;

    def renderSource(self):
        return str(self.function_name) + '(' + ','.join(map(str,self.args)) + ')'

class FunctionDecl(Node):
    """
    Members:
      functionKeyword
      functionName
      inputArgs
      outputType
    """
    def __init__(self):
        self.inputArgs = []

    def getFirstToken(self):
        return self.functionKeyword

    def renderSource(self):
        return ("function " + self.functionName.text + "(" +
            ", ".join(map(str,self.inputArgs)) + ")")



def parseStringLiteral(text):
    # the literal should have ' or " marks on either side, strip these
    return text.strip("'\"")

def getOperatorFunction(codeUnit, token):

    # Turn the token's text into a Circa string
    tokenAsString = codeUnit.createConstant(builtins.STRING_TYPE)
    ca_string.setValue(tokenAsString, token.text)

    # Find _operator function
    operatorFunc = codeUnit.getNamed('_operator')

    if operatorFunc is None:
        raise parse_errors.NoFunctionForOperator(token)

    # Access the operator function
    operatorAccessor = codeUnit.createTerm(operatorFunc, [tokenAsString])

    return operatorAccessor

def getAssignOperatorFunction(token):
    circaObj = pythonTokenToBuiltin(token)
    if circaObj is None:
        print "Notice: couldn't find an assign operator func for " + token.raw_string
        return None
    result = builtins.BUILTINS.getTerm(builtins.ASSIGN_OPERATOR_FUNC,
          inputs=[pythonTokenToBuiltin(token)])

    if result.pythonValue is None:
       return None

    return result



def testInfix():
    oneToken = tokens.TokenInstance(INTEGER, "1", 0, 0)
    twoToken = tokens.TokenInstance(INTEGER, "2", 0, 0)
    sumToken = tokens.TokenInstance(IDENT, "sum", 0, 0)
    expr = FunctionCall(sumToken, [Literal(oneToken), Literal(twoToken)])

    fakeSumFunc = Term()
    def fakeGetType():
        return None
    fakeSumFunc.getType = fakeGetType

    fakeBuilder = SpyObject()

    fakeBuilder.expectCall('createConstant(1)')
    fakeBuilder.expectCall('createConstant(2)')
    fakeBuilder.expectCall('getNamed(sum)', returnVal = fakeSumFunc)
    fakeBuilder.expectCall('createTerm(1,2)')
    
    expr.createTerms(fakeBuilder)

if __name__ == "__main__":
    testInfix()
