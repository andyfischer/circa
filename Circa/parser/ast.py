
# ast.py
#
# Defines classes for an abstract syntax tree

import parse_errors, tokens
from Circa.core import (builtins, ca_variable, ca_string, 
        ca_subroutine, ca_function, ca_type)
from Circa.core.term import Term
from Circa.common import (debug, errors)
from Circa.utils.spy_object import SpyObject
from Circa.utils.string_buffer import StringBuffer
from token_definitions import *
from Circa.core.branch import Branch

class CompilationContext(object):
    def __init__(self, codeUnit, branch, parent=None):
        debug._assert(parent is None or isinstance(parent, CompilationContext))
        debug._assert(isinstance(branch,Branch))

        self.codeUnit = codeUnit
        self.branch = branch
        self.parent = parent

    def bindName(self, term, name):
        self.branch.bindName(term,name)

    def createTerm(self, function, inputs):
        return self.codeUnit.createTerm(function, inputs, branch=self.branch)

class Node(object):
    def create(self, context):
        raise errors.PureVirtualMethodFail(self, 'create')
    def renderSource(self):
        raise errors.PureVirtualMethodFail(self, 'renderSource')
    def getFirstToken(self):
        raise errors.PureVirtualMethodFail(self, 'getFirstToken')
    def __str__(self):
        output = StringBuffer()
        self.renderSource(output)
        return str(output)

class StatementList(Node):
    def __init__(self):
        self._statements = []
    def create(self, context):
        for statement in self._statements:
            statement.create(context)
    def getFirstToken(self):
        return self._statements[0].getFirstToken()
    def renderSource(self, output):
        for statement in self._statements:
            statement.renderSource(output)
    def __iter__(self):
        for statement in self._statements:
            yield statement

    def append(self, statement):
        debug._assert(isinstance(statement, Statement))
        self._statements.append(statement)

class Statement(Node):
    pass

class IgnoredSyntax(Statement):
    def __init__(self, token):
        self.token = token
    def create(self, context):
        pass
    def renderSource(self, output):
        output.write(self.token.text)

class FunctionDeclArg(Node):
    def __init__(self, type, name):
        self.type = type
        self.name = name

    def renderSource(self, output):
        output.write(self.type.text + ' ' + self.name.text)

class FunctionDecl(Statement):
    """
    Fields:
      functionName
      inputArgs
      outputTypeIdent
      statementList

    Syntax fields:
      functionKeyword
      openParen
      closeParen
      openBracket
      closeBracket
    """

    def __init__(self):
        self.inputArgs = []

    def getFirstToken(self):
        return self.functionKeyword

    def create(self, context):

        # For each input arg, resolve the type
        inputTypeTerms = []
        for arg in self.inputArgs:
            typeTerm = context.codeUnit.getNamed(arg.type.text)
            if typeTerm is None:
                raise IdentifierNotFound(arg.type)
            inputTypeTerms.append(typeTerm)

        # Resolve output type
        outputTypeTerm = context.codeUnit.getNamed(self.outputType.text)
        if outputTypeTerm is None:
            raise IdentifierNotFound(self.outputType)

        # Create a subroutine term
        subroutineTerm = context.codeUnit.createConstant(builtins.SUBROUTINE_TYPE)

        ca_subroutine.setName(subroutineTerm, self.functionName.text)
        ca_subroutine.setInputTypes(subroutineTerm, inputTypeTerms)
        ca_subroutine.setOutputType(subroutineTerm, outputTypeTerm)

        subroutineCodeUnit = ca_subroutine.codeUnit(subroutineTerm)

        # Create placeholder terms for all inputs
        for index in range(len(self.inputArgs)):
            name = self.inputArgs[index].name.text
            typeTerm = inputTypeTerms[index]
            placeholderTerm = subroutineCodeUnit.createVariable(typeTerm)
            inputPlaceholderName = "#input_placeholder" + str(index)
            subroutineCodeUnit.bindName(placeholderTerm, inputPlaceholderName)
            subroutineCodeUnit.bindName(placeholderTerm, name)

        # Create terms for the body of the subroutine
        subsCodeUnit = ca_subroutine.codeUnit(subroutineTerm)
        innerCompilationContext = CompilationContext(
                subsCodeUnit, subsCodeUnit.mainBranch,
                parent = context)

        for statement in self.statementList:
            statement.create(innerCompilationContext)

        # Bind the subroutine's name
        context.codeUnit.bindName(subroutineTerm, self.functionName.text)
        return subroutineTerm

    def renderSource(self, output):
        output.write("function " + self.functionName.text + "(")
        firstArg = True
        for arg in self.inputArgs:
            if not firstArg:
                output.write(', ')
            firstArg = False
            arg.renderSource(output)
        output.write(") {")

        output.indent()
        for statement in self.statementList:
            statement.renderSource(output)
        output.unindent()

        output.write("}")

class ReturnStatement(Statement):
    def __init__(self, returnKeyword, right):
        self.returnKeyword = returnKeyword
        self.right = right
    def getFirstToken(self):
        return self.returnKeyword
    def create(self, context):
        result = self.right.getTerm(context)
        context.codeUnit.bindName(result, "#return_val")
    def renderSource(self, output):
        output.write('return ')
        self.right.renderSource(output)

class IfBlock(Statement):
    def __init__(self):
        self.mainBlock = None
        self.elseBlock = None
    def create(self, context):
        pass
        

class Expression(Statement):
    def create(self, context):
        return self.getTerm(context)
    def inputs(self):
        "Returns an iterable of our inputs"
        raise errors.PureVirtualMethodFail(self, 'inputs')

class Infix(Expression):
    def __init__(self, functionToken, left, right):
        assert isinstance(left, Expression)
        assert isinstance(right, Expression)

        self.token = functionToken
        self.left = left
        self.right = right

    def inputs(self):
        yield self.left
        yield self.right

    def getTerm(self, context):

        # Evaluate as an assignment?
        if self.token.match == EQUALS:
            right_term = self.right.getTerm(context)
            if not isinstance(right_term, Term):
                raise parse_errors.ExpressionDidNotEvaluateToATerm(self.right.getFirstToken())
            context.codeUnit.bindName(right_term, str(self.left))
            return right_term

        # Evaluate as feedback?
        if self.token.match == COLON_EQUALS:
            leftTerm = self.left.getTerm(context)
            rightTerm = self.right.getTerm(context)
            debug._assert(leftTerm is not None)
            debug._assert(rightTerm is not None)
            newTerm = context.createTerm(builtins.FEEDBACK_FUNC, [leftTerm, rightTerm])
            newTerm.ast = self
            newTerm.execute()
            return newTerm

        # Evaluate as a right-arrow?
        # (Not supported yet)
        """
        if self.token.match == RIGHT_ARROW:
            left_inputs = self.left.getTerm(context)
            right_func = self.right.getTerm(context)

            return context.getTerm(right_func, inputs=[left_inputs])
        """

        # Normal function?
        # Try to find a defined operator
        normalFunction = getOperatorFunction(context.codeUnit, self.token)
        if normalFunction is not None:
            debug._assert(normalFunction.cachedValue is not None)

            newTerm = context.createTerm(normalFunction,
                inputs=[self.left.getTerm(context),
                        self.right.getTerm(context)])
            newTerm.ast = self
            return newTerm

        # Evaluate as a function + assign?
        # Try to find an assign operator
        assignFunction = getAssignOperatorFunction(self.token.match)
        if assignFunction is not None:
            # create a term that's the result of the operation
            result_term = context.createTerm(assignFunction,
               inputs=[self.left.getTerm(context), self.right.getTerm(context)])

            # bind the name to this result
            context.codeUnit.bindName(result_term, str(self.left))
            return result_term

        debug.fail("Unable to evaluate token: " + self.token.text)

    def getFirstToken(self):
        return self.left.getFirstToken()

    def renderSource(self, output):
        self.left.renderSource(output)
        output.write(' ' + self.token.text + ' ')
        self.right.renderSource(output)

class LiteralString(Expression):
    def __init__(self, token):
        self.token = token
        self.hasQuestionMark = False

    def inputs(self):
        return []

    def getTerm(self, context):
        term = context.codeUnit.createVariable(builtins.STRING_TYPE)
        term.ast = self
        # Strip quotation marks
        value = self.token.text.strip("'\"")
        # Convert \n to newline
        value = value.replace("\\n","\n")
        ca_variable.setValue(term, value)
        return term

    def renderSource(self,output):
        output.write(self.token.text)

class Literal(Expression):
    def __init__(self, token, hasQuestionMark=False):
        self.token = token
        self.hasQuestionMark = hasQuestionMark

        if token.match == FLOAT:
            self.value = float(token.text)
            self.circaType = builtins.FLOAT_TYPE
        elif token.match == INTEGER:
            self.value = int(token.text)
            self.circaType = builtins.INT_TYPE
            """
        elif token.match == STRING:
            # the literal should have ' or " marks on either side, strip these
            self.value = token.text.strip("'\"")
            self.circaType = builtins.STRING_TYPE
            """
        elif token.match == MULTILINE_STR:
            self.value = token.text[3:-3]
            self.circaType = builtins.STRING_TYPE
        else:
            raise parse_errors.InternalError("Couldn't recognize token: " + str(token))

    def inputs(self):
        return []

    def getTerm(self, context):
        # Create a term
        newTerm = context.codeUnit.createVariable(self.circaType)
        newTerm.ast = self

        # Assign value
        ca_variable.setValue(newTerm, self.value)
        return newTerm

    def getFirstToken(self):
        return self.token

    def renderSource(self, output):
        if (self.token.match == STRING):
            output.write("'" + self.value + "'")
        else:
            output.write(str(self.value))

class Ident(Expression):
    def __init__(self, token):
        self.token = token

    def inputs(self):
        return []

    def getTerm(self, context):
        term = context.codeUnit.getNamed(self.token.text)

        if not term:
            raise parse_errors.IdentifierNotFound(self.token)

        return context.codeUnit.getNamed(self.token.text)

    def getFirstToken(self):
        return self.token

    def renderSource(self, output):
        output.write(self.token.text)

class Unary(Expression):
    def __init__(self, functionToken, right):
        debug._assert(isinstance(right, Expression))

        self.functionToken = functionToken
        self.right = right

    def inputs(self):
        yield self.right

    def getTerm(self, context):
        mult = context.codeUnit.getNamed('mult')
        negative_one = context.codeUnit.createConstant(builtins.INT_TYPE)
        ca_variable.setValue(negative_one, -1)
        newTerm = context.createTerm(mult,
                   inputs = [negative_one, self.right.getTerm(context)])
        newTerm.ast = self
        return newTerm

    def getFirstToken(self):
        return self.functionToken;

    def renderSource(self, output):
        output.write('-')
        self.right.renderSource(output)

class FunctionCall(Expression):
    def __init__(self, function_name, args):
        for arg in args:
            debug._assert(isinstance(arg,Expression))

        self.function_name = function_name
        self.args = args

    def inputs(self):
        for arg in self.args:
            yield arg

    def getTerm(self, context):
        arg_terms = [expr.getTerm(context) for expr in self.args]
        func = context.codeUnit.getNamed(self.function_name.text)

        if func is None:
            raise parse_errors.InternalError(self.function_name,
                "Function " + self.function_name.text + " not found.")

        # Check for Function
        if func.getType() in (builtins.FUNCTION_TYPE, builtins.SUBROUTINE_TYPE):
            newTerm = context.createTerm(func, inputs=arg_terms)
            newTerm.ast = self
            return newTerm

        # Temp: Use a Python dynamic type check to see if this is a function
        elif isinstance(func.pythonValue, ca_function._Function):
            print 'FunctionCall - deprecated stuff'
            return context.createTerm(func, inputs=arg_terms)

        else:
            raise parse_errors.InternalError(self.function_name,
               "Term " + self.function_name.text + " is not a function.")

    def getFirstToken(self):
        return self.function_name;

    def renderSource(self, output):
        output.write(str(self.function_name) + '(' + ','.join(map(str,self.args)) + ')')


def getOperatorFunction(codeUnit, token):

    # Turn the token's text into a Circa string
    tokenAsString = codeUnit.createConstant(builtins.STRING_TYPE)
    ca_string.setValue(tokenAsString, token.text)

    # Find operator function
    operatorFunc = codeUnit.getNamed('operator')

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
    
    expr.create(fakeBuilder)

if __name__ == "__main__":
    testInfix()
