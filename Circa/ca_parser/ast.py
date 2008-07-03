#
# ast.py
#
# Defines classes for an abstract syntax tree

import pdb

import parse_errors, tokens
from Circa.core import (builtins, ca_codeunit, ca_variable, ca_string, 
        ca_subroutine, ca_function, ca_type)
from Circa.runtime import ca_module
from Circa.core.term import Term
from Circa.common import (debug, errors, term_syntax)
from Circa.utils.spy_object import SpyObject
from Circa.utils.string_buffer import StringBuffer
from token_definitions import *
from Circa.core.branch import Branch
import Circa

class GlobalCompilationContext(object):
    def getNamed(self, name):
        for module in Circa.LOADED_MODULES.values():
            if module.containsName(name):
                return module.getNamed(name)

class CompilationContext(object):
    def __init__(self, codeUnit, parent=None, branch=None):
        debug._assert(parent is None or isinstance(parent,CompilationContext))
        debug._assert(branch is None or isinstance(branch,Branch))

        if branch is None:
            branch = codeUnit.mainBranch

        if parent is None:
            parent = GlobalCompilationContext()

        self.codeUnit = codeUnit
        self.branch = branch
        self.parent = parent
        self.rebinds = {}
        self.metaOptionHandler = None

    def bindName(self, term, name):
        # Check if this name is already defined
        if self.branch.containsName(name):
            self.branch.bindName(term,name,allowOverwrite=True)
            # Do something else here
        else:
            self.branch.bindName(term,name)

    def getNamed(self, name):
        if self.branch.containsName(name):
            return self.branch.getNamed(name)

        if self.parent is not None:
            return self.parent.getNamed(name)

        return None

    def createTerm(self, function, inputs):
        return self.codeUnit.createTerm(function, inputs, branch=self.branch)

    def createConstant(self, type):
        return self.codeUnit.createConstant(type, branch=self.branch)

    def createVariable(self, type):
        return self.codeUnit.createVariable(type, branch=self.branch)

    def handleMetaOption(self, optionName):
        if self.metaOptionHandler is None:
            raise parse_errors.NoOptionHandler()
        else:
            self.metaOptionHandler(optionName)

class CompilationUnit(object):
    def __init__(self, statementList):
        self.statementList = statementList

    def createModule(self):
        codeUnit = ca_codeunit.CodeUnit()
        resultModule = ca_module.Module("name", codeUnit)
        compilationContext = CompilationContext(codeUnit)

        def handleMetaOption(option):
            if option == "mutable-file":
                ca_module.mutableFile = True
            else:
                raise Exception("Unrecognized option: " + option)
        compilationContext.metaOptionHandler = handleMetaOption
        
        self.statementList.create(compilationContext)

        return resultModule

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

class CommentLine(Statement):
    def __init__(self, text):
        self.text = text
    def create(self, context):
        term = context.createTerm(builtins.COMMENT_FUNC, [])
        term.termSyntaxInfo = term_syntax.CommentLine(self.text)
        return term
    def renderSource(self, output):
        output.write(self.text)

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
            typeTerm = context.getNamed(arg.type.text)
            if typeTerm is None:
                raise IdentifierNotFound(arg.type)
            inputTypeTerms.append(typeTerm)

        # Resolve output type
        if self.outputType is None:
            outputTypeTerm = builtins.VOID_TYPE
        else:
            outputTypeTerm = context.getNamed(self.outputType.text)
        if outputTypeTerm is None:
            raise IdentifierNotFound(self.outputType)

        # Create a subroutine term
        subroutineTerm = context.createConstant(builtins.SUBROUTINE_TYPE)

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
            placeholderTerm.outputReady = False

        # Create terms for the body of the subroutine
        subsCodeUnit = ca_subroutine.codeUnit(subroutineTerm)
        innerCompilationContext = CompilationContext(
                subsCodeUnit, parent = context)

        for statement in self.statementList:
            statement.create(innerCompilationContext)

        # Bind the subroutine's name
        context.bindName(subroutineTerm, self.functionName.text)

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
        (result,ast) = self.right.getTerm(context)
        result.termSyntaxInfo = term_syntax.Expression(ast, '#return_val')
        context.bindName(result, '#return_val')
    def renderSource(self, output):
        output.write('return ')
        self.right.renderSource(output)

class IfBlock(Statement):
    def __init__(self, condition, mainBlock, elseBlock):
        debug._assert(isinstance(condition,Expression))
        debug._assert(isinstance(mainBlock,StatementList))
        debug._assert(elseBlock is None or isinstance(elseBlock,StatementList))

        self.condition = condition
        self.mainBlock = mainBlock
        self.elseBlock = elseBlock

    def create(self, context):

        # Create term for condition
        condition = self.condition.create(context)

        # Create an if-statement term
        ifStatement = context.createTerm(builtins.IF_STATEMENT, [condition])

        ifStatement.state.branches.append(Branch(ifStatement))
        self.mainBlock.create(CompilationContext(context.codeUnit, context,
            ifStatement.state.branches[0]))

        if self.elseBlock is not None:
            ifStatement.state.branches.append(Branch(ifStatement))
            self.elseBlock.create(CompilationContext(context.codeUnit, context,
                ifStatement.state.branches[1]))

        # Get a set of all names that are defined within our branches, and
        # are also defined outside.
        namesToJoin = set()
        for branch in ifStatement.state.branches:
            for (name,t) in branch.iterateNamespace():
                if context.getNamed(name) is not None:
                    namesToJoin.add(name)

        # For every name to join, figure out the branch head. This will
        # either be:
        #  1) the binding from within this branch
        #  or 2) the binding from outside, if the branch doesn't bind it

        heads = {} # maps names to lists
        for name in namesToJoin:
            heads[name] = []

            for branch in ifStatement.state.branches:
                if branch.containsName(name):
                    heads[name].append(branch.getNamed(name))
                else:
                    heads[name].append(context.getNamed(name))

            # If there's no else block, add original heads
            if len(ifStatement.state.branches) == 1:
                heads[name].append(context.getNamed(name))

        # Create if-expr terms to join heads, and rebind the name to this
        # new term.
        for (name,head_list) in heads.items():
            inputs = [condition] + head_list
            term = context.createTerm(builtins.IF_EXPR, inputs)
            context.bindName(term, name)

        return ifStatement

    def renderSource(self,output):
        output.write('if (')
        self.condition.renderSource(output)
        output.write(') {')
        self.mainBlock.renderSource(output)
        output.write('}')
        if self.elseBlock is not None:
            output.write(' else {')
            output.elseBlock.renderSource(output)
            output.write('}')

class WhileBlock(Statement):
    def __init__(self, condition, stmtList):
        debug._assert(isinstance(condition,Expression))
        debug._assert(isinstance(stmtList,StatementList))

        self.condition = condition
        self.statementList = stmtList

class NameBinding(Statement):
    def __init__(self, left, right):
        debug._assert(isinstance(left, tokens.TokenInstance))
        debug._assert(isinstance(right, Expression))
        self.left = left
        self.right = right

    def create(self, context):
        (rightTerm, ast) = self.right.getTerm(context)
        name = self.left.text
        if not isinstance(rightTerm, Term):
            raise parse_errors.ExpressionDidNotEvaluateToATerm(self.right.getFirstToken())

        rightTerm.termSyntaxInfo = term_syntax.Expression(ast, name)
        context.bindName(rightTerm, name)
        return rightTerm

class HighLevelOptionStatement(Statement):
    def __init__(self, optionName):
        debug._assert(isinstance(optionName, tokens.TokenInstance))
        self.optionName = optionName
    def create(self, context):
        context.handleMetaOption(self.optionName.text)

        # Create a term to represent this high level option
        option = context.createTerm(builtins.HIGH_LEVEL_OPTION_FUNC, inputs=[])
        option.state = self.optionName.text
        option.termSyntaxInfo = term_syntax.HighLevelOption(self.optionName.text)
        return option

class Expression(Statement):
    def create(self, context):
        (term,ast) = self.getTerm(context)
        term.termSyntaxInfo = term_syntax.Expression(ast)
        return term

    def inputs(self):
        "Returns an iterable of our inputs"
        raise errors.PureVirtualMethodFail(self, 'inputs')

    def getTerm(self, context):
        """
        Creates a term (or finds an existing one). Returns a tuple of
        (term, term_syntax.Ast)
        """
        raise errors.PureVirtualMethodFail(self, 'inputs')


class Infix(Expression):
    def __init__(self, functionToken, left, right):
        debug._assert(isinstance(left, Expression))
        debug._assert(isinstance(right, Expression))

        self.token = functionToken
        self.left = left
        self.right = right

    def inputs(self):
        yield self.left
        yield self.right

    def getTerm(self, context):

        # Evaluate as feedback?
        if self.token.match == COLON_EQUALS:
            (leftTerm, leftAst) = self.left.getTerm(context)
            (rightTerm, rightAst) = self.right.getTerm(context)
            debug._assert(leftTerm is not None)
            debug._assert(rightTerm is not None)
            newTerm = context.createTerm(builtins.FEEDBACK_FUNC, [leftTerm, rightTerm])
            newTerm.execute()
            return (newTerm, term_syntax.Infix(':=', leftAst, rightAst))

        # Evaluate as a right-arrow?
        # (Not supported yet)
        """
        if self.token.match == RIGHT_ARROW:
            left_inputs = self.left.getTerm(context)
            right_func = self.right.getTerm(context)

            return context.getTerm(right_func, inputs=[left_inputs])
        """

        # Evaluate as a dotted expression?
        if self.token.match == DOT:
            debug._assert(isinstance(self.right, Ident))
            (leftTerm, leftAst) = self.left.getTerm(context)

            rightTermAsString = context.createConstant(builtins.STRING_TYPE)
            ca_variable.setValue(rightTermAsString, self.right.token.text)

            newTerm = context.createTerm(builtins.GET_FIELD,
                    inputs=[leftTerm, rightTermAsString])
            return (newTerm, None)

        # Normal function?
        # Try to find a defined operator
        normalFunction = getOperatorFunction(context, self.token)
        if normalFunction is not None:
            debug._assert(normalFunction.cachedValue is not None)
            (leftTerm, leftAst) = self.left.getTerm(context)
            (rightTerm, rightAst) = self.right.getTerm(context)

            newTerm = context.createTerm(normalFunction,
                inputs=[leftTerm, rightTerm])
            return (newTerm, term_syntax.Infix(self.token.text,leftAst,rightAst))

        # Evaluate as a function + assign?
        # Try to find an assign operator
        assignFunction = getAssignOperatorFunction(self.token.match)
        if assignFunction is not None:
            (leftTerm, leftAst) = self.left.getTerm(context)
            (rightTerm, rightAst) = self.right.getTerm(context)

            # create a term that's the result of the operation
            result_term = context.createTerm(assignFunction,
               inputs=[leftTerm,rightTerm])

            # bind the name to this result
            context.bindName(result_term, str(self.left))
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
        term = context.createVariable(builtins.STRING_TYPE)
        # Strip quotation marks
        value = self.token.text[1:-1]
        # Convert \n to newline
        value = value.replace("\\n","\n")
        ca_variable.setValue(term, value)
        return (term, term_syntax.TermValue(term))

    def renderSource(self,output):
        output.write(self.token.text)

class ListExpr(Expression):
    def __init__(self):
        self.elements = []
    def append(self, expr):
        debug._assert(isinstance(expr,Expression))
        self.elements.append(expr)
    def getTerm(self, context):
        terms = []
        for element in self.elements:
            t = element.getTerm(context)
            terms.append(t)

        return context.createTerm(builtins.PACK_LIST_FUNC, terms)

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
        newTerm = context.createVariable(self.circaType)

        # Assign value
        ca_variable.setValue(newTerm, self.value)
        return (newTerm, term_syntax.TermValue(newTerm))

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
        name = self.token.text
        term = context.getNamed(name)

        if not term:
            raise parse_errors.IdentifierNotFound(self.token)

        return (term, term_syntax.TermName(term, name))

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
        mult = context.getNamed('mult')
        negative_one = context.codeUnit.createConstant(builtins.INT_TYPE)
        ca_variable.setValue(negative_one, -1)
        (rightTerm, rightAst) = self.right.getTerm(context)
        newTerm = context.createTerm(mult,
                   inputs = [negative_one, rightTerm])
        newTerm.ast = self
        return (newTerm, term_syntax.Unary('-', rightAst))

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
        argTerms = []
        argAsts = []
        for expr in self.args:
            (term, ast) = expr.getTerm(context)
            argTerms.append(term)
            argAsts.append(ast)

        func = context.getNamed(self.function_name.text)

        if func is None:
            raise parse_errors.InternalError(self.function_name,
                "Function " + self.function_name.text + " not found.")

        # Check for Function
        if func.getType() in (builtins.FUNCTION_TYPE, builtins.SUBROUTINE_TYPE):
            try:
                newTerm = context.createTerm(func, inputs=argTerms)
                return (newTerm, term_syntax.FunctionCall(self.function_name.text, argAsts))
            except errors.CircaError,e:
                pdb.set_trace()
                raise parse_errors.InternalError(self.function_name, str(e))

        else:
            raise parse_errors.InternalError(self.function_name,
               "Term " + self.function_name.text + " is not a function.")

    def getFirstToken(self):
        return self.function_name;

    def renderSource(self, output):
        output.write(str(self.function_name) + '(' + ','.join(map(str,self.args)) + ')')


def getOperatorFunction(context, token):

    # Turn the token's text into a Circa string
    tokenAsString = context.createConstant(builtins.STRING_TYPE)
    ca_string.setValue(tokenAsString, token.text)

    # Find operator function
    operatorFunc = context.getNamed('operator')

    if operatorFunc is None:
        raise parse_errors.NoFunctionForOperator(token)

    # Access the operator function
    operatorAccessor = context.createTerm(operatorFunc, [tokenAsString])

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
