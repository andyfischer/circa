#
# parser.py
#
# High-level parser, turns a string into a Circa code object.
#

from Circa.core import ca_codeunit
from Circa.parser import ast
import tokens as _tokens_module
from token_definitions import *
import parse_errors

def parseFile(filename, compilationCU=None):
    """
    Parse the given file.
    Returns tuple: (errors, CodeUnit)
       errors is a list of ParseErrors, or an empty list (if none occured)
       codeUnit is an instance of CodeUnit, or None if errors occured
    """

    fileReader = open(filename, 'r')
    fileContents = fileReader.read()
    fileReader.close()

    tokens = _tokens_module.tokenize(fileContents)

    # Scan the token stream for an instance of UNRECOGNIZED token
    # If found, immediately return with this error
    while not tokens.finished():
        token = tokens.consume()
        if token.match == UNRECOGNIZED:
            return ([parse_errors.UnrecognizedToken(token)], None)

    # Reset token stream
    tokens.reset()

    pstate = ParserState(tokens, ca_codeunit.CodeUnit(),
            compilationCU=compilationCU)

    compilationContext = ast.CompilationContext(pstate.codeUnit, pstate.compilationCU)

    try:
        resultAst = statement_list(pstate.tokens)
        resultAst.createTerms(compilationContext)
        pstate.codeUnit.ast = resultAst

    except parse_errors.ParseError, e:
        pstate.errors.append(e)

    return (pstate.errors, pstate.codeUnit)


class ParserState(object):
    def __init__(self, tokenSource, resultCodeUnit, compilationCU=None):
        self.tokens = tokenSource
        self.errors = []
        self.codeUnit = resultCodeUnit

        if compilationCU is None:
            self.compilationCU = resultCodeUnit
        else:
            self.compilationCU = compilationCU


def expression_statement(pstate):
    try:
        mark = pstate.tokens.markLocation()
        resultAst = parseExpression(pstate.tokens)
        term = resultAst.createTerms(
            ast.CompilationContext(pstate.codeUnit, pstate.compilationCU))

        pstate.codeUnit.statementAsts.append(resultAst)

    except MatchFailed, e:
        pstate.tokens.restoreMark(mark)
        raise parse_errors.ExpectedExpression(pstate.tokens.next())

def statement_list(tokens):
    """
    Parses a list of statements. Stops when we encounter a } or reach the
    end of the stream.
    """

    result = ast.StatementList()

    while not tokens.nextIs(RBRACKET) and not tokens.finished():
        result.statements.append(statement(tokens))

    return result

def statement(tokens):
    if tokens.nextIs(FUNCTION):
        return function_decl(tokens)
    elif tokens.nextIs(COMMENT_LINE):
        return ast.IgnoredSyntax(tokens.consume(COMMENT_LINE))
    elif tokens.nextIs(NEWLINE):
        return ast.IgnoredSyntax(tokens.consume(NEWLINE))
    elif tokens.nextIs(RETURN):
        return return_statement(tokens)
    else:
        return infix_expression(tokens, 0)


# Infix precedence
HIGHEST_INFIX_PRECEDENCE = 7
_infixPrecedence = {
    DOT: 7,
    STAR: 6, SLASH: 6,
    PLUS: 5, MINUS: 5,
    LTHAN: 3, LTHANEQ: 3, GTHAN: 3, GTHANEQ: 3, DOUBLE_EQUALS: 3, NOT_EQUALS: 3,
    EQUALS: 2, PLUS_EQUALS: 2, MINUS_EQUALS: 2, STAR_EQUALS: 2, SLASH_EQUALS: 2,
      COLON_EQUALS: 2,
    RIGHT_ARROW: 1 
}

def _getInfixPrecedence(token):
    try:
        return _infixPrecedence[token.match]
    except KeyError:
        return -1

def infix_expression(tokens, precedence):
    if (precedence > HIGHEST_INFIX_PRECEDENCE):
       return unary_expression(tokens)

    expr = infix_expression(tokens, precedence + 1)
    if not expr: return None

    while tokens.next() and _getInfixPrecedence(tokens.next()) == precedence:
        operator = tokens.consume()

        first_righthand_token = tokens.next()
        right_expr = infix_expression(tokens, precedence + 1)

        if not right_expr:
            raise parse_errors.InternalError(first_righthand_token)

        expr = ast.Infix(operator, expr, right_expr)

    return expr

def unary_expression(tokens):
    if tokens.nextIs(MINUS):
        minus = tokens.consume(MINUS)
        return Unary(minus, atom(tokens))
    else:
        return atom(tokens)

def atom(tokens):
    # Function call
    if tokens.nextIs(IDENT) and tokens.nextIs(LPAREN, lookahead=1):
        return function_call(tokens)

    # Literal value
    if tokens.nextIn((FLOAT, INTEGER, STRING)):
        token = tokens.consume()
        questionMark = False
        if tokens.nextIs(QUESTION):
            tokens.consume(QUESTION)
            questionMark = True
        return ast.Literal(token, hasQuestionMark=questionMark)

    # Identifier
    if tokens.nextIs(IDENT):
        token = tokens.consume()
        return ast.Ident(token)

    # Parenthesized expression
    if tokens.nextIs(LPAREN):
        tokens.consume(LPAREN)
        expr = infix_expression(tokens, 0)
        tokens.consume(RPAREN)
        return expr
  
    # Failed to match
    if tokens.finished():
        raise parse_errors.UnexpectedEOF()

    raise parse_errors.UnexpectedToken(tokens.next())
 
def function_call(tokens):
    function_name = tokens.consume(IDENT)
    tokens.consume(LPAREN)

    args = []

    if not tokens.nextIs(RPAREN):
        args.append( infix_expression(tokens, 0) )

        while tokens.nextIs(COMMA):
            tokens.consume(COMMA)
            args.append( infix_expression(tokens, 0) )

    tokens.consume(RPAREN)

    return ast.FunctionCall(function_name, args)

def function_decl(tokens):
    result = ast.FunctionDecl()

    result.functionKeyword = tokens.consume(FUNCTION)

    result.functionName = tokens.consume(IDENT)
    result.openParen = tokens.consume(LPAREN)

    while not tokens.nextIs(RPAREN):
        argType = tokens.consume(IDENT)
        argName = tokens.consume(IDENT)
        result.inputArgs.append( ast.FunctionDeclArg(argType, argName) )

        if not tokens.nextIs(COMMA):
            break
        else:
            tokens.consume(COMMA)

    result.closeParen = tokens.consume(RPAREN)

    if tokens.nextIs(RIGHT_ARROW):
        tokens.consume(RIGHT_ARROW)
        result.outputType = tokens.consume(IDENT)
    else:
        result.outputType = None

    result.openBracket = tokens.consume(LBRACKET)

    result.statementList = statement_list(tokens)

    result.closeBracket = tokens.consume(RBRACKET)
    
    return result

def return_statement(tokens):
    returnStmt = ast.ReturnStatement()
    returnStmt.returnKeyword = tokens.consume(RETURN)
    returnStmt.expr = infix_expression(tokens, 0)
    return returnStmt

def testEquals():
    import tokens
    ts = tokens.tokenize("a = 1")
    print infix_expression(ts, 0)

if __name__ == '__main__':
    testEquals()

