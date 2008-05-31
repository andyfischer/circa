
# ast.py
#
# Defines classes for an abstract syntax tree

import parse_errors, tokens
from Circa import debug
from Circa.core import builtins
from Circa.core.term import Term
from Circa.debug.spy_object import SpyObject

class Node(object):
    pass

class Infix(Node):
    def __init__(self, functionToken, left, right):
        assert isinstance(left, Node)
        assert isinstance(right, Node)

        self.token = functionToken
        self.left = left
        self.right = right

    def eval(self, builder):

        # Evaluate as an assignment?
        if self.token.match == tokens.EQUALS:
            right_term = self.right.eval(builder)
            if not isinstance(right_term, Term):
                raise parse_errors.ExpressionDidNotEvaluateToATerm(self.right.getFirstToken())
            return builder.bindName(str(self.left), right_term)

        # Evaluate as a right-arrow?
        if self.token.match == tokens.RIGHT_ARROW:
            left_inputs = self.left.eval(builder)
            right_func = self.right.eval(builder)

            return builder.createTerm(right_func, inputs=[left_inputs])

        # Normal function?
        # Try to find a defined operator
        normalFunction = getOperatorFunction(self.token.match)
        if normalFunction is not None:
            assert normalFunction.pythonValue is not None

            return builder.createTerm(normalFunction,
                inputs=[self.left.eval(builder), self.right.eval(builder)] )

        # Evaluate as a function + assign?
        # Try to find an assign operator
        assignFunction = getAssignOperatorFunction(self.token.match)
        if assignFunction is not None:
            # create a term that's the result of the operation
            result_term = builder.createTerm(assignFunction,
               inputs=[self.left.eval(builder), self.right.eval(builder)])

            # bind the name to this result
            return builder.bindName(str(self.left), result_term)

        debug.fail("Unable to evaluate token: " + self.token.text)

    def getFirstToken(self):
        return self.left.getFirstToken()

    def __str__(self):
        return self.function.text + "(" + str(self.left) + "," + str(self.right) + ")"

class Literal(Node):
    def __init__(self, token, hasQuestionMark=False):
        self.token = token
        self.hasQuestionMark = hasQuestionMark

        if token.match == tokens.FLOAT:
            self.value = float(token.text)
        elif token.match == tokens.INTEGER:
            self.value = int(token.text)
        elif token.match == tokens.STRING:
            self.value = parseStringLiteral(token.text)
        else:
            raise parse_errors.InternalError("Couldn't recognize token: " + str(token))

    def eval(self, builder):
        if self.hasQuestionMark:
            return builder.createVariable(self.value, sourceToken=self.token)
        else:
            return builder.createConstant(self.value, sourceToken=self.token)

    def getFirstToken(self):
        return self.token

    def __str__(self):
        return str(self.value)

class Ident(Node):
    def __init__(self, token):
        self.token = token

    def eval(self, builder):
        term = builder.getNamed(self.token.text)

        if not term:
            raise parse_errors.IdentifierNotFound(self.token)

        return builder.getNamed(self.token.text)

    def getFirstToken(self):
        return self.token

    def __str__(self):
       return self.token.text

class Unary(Node):
    def __init__(self, functionToken, right):
        self.functionToken = functionToken
        self.right = right

    def eval(self, builder):
        return builder.getTerm(builtins.MULT,
                               inputs = [builder.createConstant(-1),
                                         self.right.eval(builder)])

    def getFirstToken(self):
        return self.functionToken;

    def __str__(self):
        return self.functionToken.text + "(" + str(self.right) + ")"

class FunctionCall(Node):
    def __init__(self, function_name, args):
        self.function_name = function_name
        self.args = args

    def eval(self, builder):
        arg_terms = [t.eval(builder) for t in self.args]
        func = builder.getNamed(self.function_name.text)

        if func is None:
            raise parse_errors.InternalError(self.function_name,
              "Function " + self.function_name.text + " not found.")

        # Check for Function
        if func.getType() is builtins.FUNCTION_TYPE:
            return builder.createTerm(func, inputs=arg_terms)

        # Check for Subroutine
        elif func.getType() is builtins.SUBROUTINE_TYPE:

            # Todo: special behavior for invoking subroutines
            return builder.createTerm(builtins.INVOKE_SUB_FUNC)

        # Temp: Use a Python dynamic type check to see if this is a function
        elif isinstance(func.pythonValue, ca_function._Function):
            return builder.createTerm(func, inputs=arg_terms)

        else:
            raise parse_errors.InternalError(self.function_name,
               "Term " + self.function_name.text + " is not a function.")

    def getFirstToken(self):
        return self.function_name;

    def __str__(self):
        return self.function_name + '(' + ','.join(map(str,self.args)) + ')'

def testInfix():
    oneToken = tokens.TokenInstance(tokens.INTEGER, "1", 0, 0)
    twoToken = tokens.TokenInstance(tokens.INTEGER, "2", 0, 0)
    sumToken = tokens.TokenInstance(tokens.IDENT, "sum", 0, 0)
    expr = FunctionCall(sumToken, [Literal(oneToken), Literal(twoToken)])

    fakeSumFunc = Term()
    def fakeGetType():
        return None
    fakeSumFunc.getType = fakeGetType

    fakeBuilder = SpyObject()

    fakeBuilder.expectCall('createConstant(1,sourceToken=1)')
    fakeBuilder.expectCall('createConstant(2,sourceToken=2)')
    fakeBuilder.expectCall('getNamed(sum)', returnVal = fakeSumFunc)
    fakeBuilder.expectCall('createTerm(1,2)')
    
    expr.eval(fakeBuilder)

if __name__ == "__main__":
    testInfix()
