
# ast.py
#
# Defines classes for an abstract syntax tree

class Node(object):
    def eval(self, builder):
        raise Exception("Need to implement this")

    def getFirstToken(self):
        raise Exception("Need to implement this")

class Infix(Node):
    def __init__(self, function_token, left, right):
        assert isinstance(function_token, token.Token)
        assert isinstance(left, Node)
        assert isinstance(right, Node)

        self.token = function_token
        self.left = left
        self.right = right

    def eval(self, builder):

        # Evaluate as an assignment?
        if self.token.match == EQUALS:
            right_term = self.right.eval(builder)
            if not isinstance(right_term, code.Term):
               raise parse_errors.ExpressionDidNotEvaluateToATerm(self.right.getFirstToken())
            return builder.bindName(self.left.getName(), right_term)

        # Evaluate as a right-arrow?
        if self.token.match == RIGHT_ARROW:
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
            return builder.bindName(self.left.getName(), result_term)

        pdb.set_trace()
        raise Exception("Unable to evaluate token: " + self.token.text)

    def getFirstToken(self):
        return self.left.getFirstToken()

    def __str__(self):
        return self.function.text + "(" + str(self.left) + "," + str(self.right) + ")"

class Literal(Node):
    def __init__(self, token, hasQuestionMark=False):
        self.token = token
        self.hasQuestionMark = hasQuestionMark

        if token.match == FLOAT:
            self.value = float(token.text)
        elif token.match == INTEGER:
            self.value = int(token.text)
        elif token.match == STRING:
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

    def getName(self):
       return self.token.text

    def __str__(self):
       return self.token.text

class Unary(Node):
    def __init__(self, function_token, right):
        self.function_token = function_token
        self.right = right

    def eval(self, builder):
        return builder.getTerm(builtins.MULT,
                               inputs = [builder.createConstant(-1),
                                         self.right.eval(builder)])

    def getFirstToken(self):
        return self.function_token;

    def __str__(self):
        return self.function_token.text + "(" + str(self.right) + ")"

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
        if func.getType() is builtins.FUNC_TYPE:
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
