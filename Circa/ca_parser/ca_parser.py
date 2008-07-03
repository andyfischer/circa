#
# parser.py
#
# High-level parser, turns a string into a Circa code object.
#

import pdb
from Circa.core import ca_codeunit
from Circa.ca_parser import ast
import tokens as _tokens_module
from token_definitions import *
import parse_errors

def parseFile(filename):
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

    errors = []

    resultModule = None
    try:
    
    # Compile to an AST
        compilationUnit = compilation_unit(tokens)
        resultModule = compilationUnit.createModule()

        resultModule.filename = filename

    except parse_errors.ParseError, e:
        errors.append(e)

    return (errors, resultModule)


class ParserState(object):
    def __init__(self, tokenSource, resultCodeUnit):
        self.tokens = tokenSource
        self.errors = []
        self.codeUnit = resultCodeUnit


def expression_statement(pstate):
    try:
        mark = pstate.tokens.markLocation()
        resultAst = parseExpression(pstate.tokens)
        term = resultAst.create(
            ast.CompilationContext(pstate.codeUnit))

        pstate.codeUnit.statementAsts.append(resultAst)

    except MatchFailed, e:
        pstate.tokens.restoreMark(mark)
        raise parse_errors.ExpectedExpression(pstate.tokens.next())

def compilation_unit(tokens):
    return ast.CompilationUnit(statement_list(tokens))

def statement_list(tokens):
    """
    Parses a list of statements. Stops when we encounter a } or reach the
    end of the stream.
    """

    result = ast.StatementList()

    while not tokens.nextIs(RBRACKET) and not tokens.finished():
        result.append(statement(tokens))

    return result

def statement(tokens):
    if tokens.nextIs(FUNCTION):
        return function_decl(tokens)
    elif tokens.nextIs(COMMENT_LINE):
        commentLine = tokens.consume(COMMENT_LINE)
        return ast.CommentLine(commentLine.text)
    elif tokens.nextIs(NEWLINE):
        tokens.consume(NEWLINE)
        return ast.CommentLine("")
    elif tokens.nextIs(RETURN):
        return return_statement(tokens)
    elif tokens.nextIs(IF):
        return if_statement(tokens)
    elif tokens.nextIs(LBRACE):
        return high_level_option_statement(tokens)

    # Lookahead, check if this is a name binding
    if tokens.nextIs(IDENT) and tokens.nextIs(EQUALS, lookahead=1):
        return name_binding_statement(tokens)

    # Otherwise, evaluate as an expression
    return expression_statement(tokens)


def expression_statement(tokens):
    expr = infix_expression(tokens)
    statement_end(tokens)
    return expr

def name_binding_statement(tokens):
    name = tokens.consume(IDENT)
    equalsSign = tokens.consume(EQUALS)
    rightExpr = infix_expression(tokens)
    statement_end(tokens)
    return ast.NameBinding(name, rightExpr)

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

def infix_expression(tokens, precedence=0):
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
        return ast.Unary(minus, atom(tokens))
    else:
        return atom(tokens)

def atom(tokens):
    # Function call
    if tokens.nextIs(IDENT) and tokens.nextIs(LPAREN, lookahead=1):
        return function_call(tokens)

    # Literal string
    if tokens.nextIs(STRING):
        token = tokens.consume()
        questionMark = optional_question_mark(tokens)
        return ast.LiteralString(token)

    # Literal value
    if tokens.nextIn((FLOAT, INTEGER, MULTILINE_STR)):
        token = tokens.consume()
        questionMark = optional_question_mark(tokens)
        return ast.Literal(token)

    # List expression (surrounded by []s)
    # Disabled
    #if tokens.nextIs(LBRACE):
    #    return list_expr(tokens)

    # Identifier
    if tokens.nextIs(IDENT):
        token = tokens.consume()
        return ast.Ident(token)

    # Parenthesized expression
    if tokens.nextIs(LPAREN):
        tokens.consume(LPAREN)
        expr = infix_expression(tokens)
        tokens.consume(RPAREN)
        return expr
  
    # Failed to match
    if tokens.finished():
        raise parse_errors.UnexpectedEOF()

    pdb.set_trace()
    raise parse_errors.UnexpectedToken(tokens.next())

def optional_question_mark(tokens):
    """
    This function checks if the next token is a question mark. If so,
    consume and return it. If not, returns None.
    """
    if tokens.nextIs(QUESTION):
        return tokens.consume(QUESTION)
    return None
 
def function_call(tokens):
    function_name = tokens.consume(IDENT)
    tokens.consume(LPAREN)

    wasSkippingNewline = tokens.isSkipping(NEWLINE)
    tokens.startSkipping(NEWLINE)

    args = []

    if not tokens.nextIs(RPAREN):
        args.append( infix_expression(tokens) )

        while tokens.nextIs(COMMA):
            tokens.consume(COMMA)
            args.append( infix_expression(tokens) )

    if not wasSkippingNewline:
        tokens.stopSkipping(NEWLINE)
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

def list_expr(tokens):
    lbrace = tokens.consume(LBRACE)
    result = ast.ListExpr

    while not tokens.nextIs(RBRACE):
        result.append(infix_expression(tokens))

        if not tokens.nextIs(COMMA):
            break
        
        tokens.consume(COMMA)

    rbrace = tokens.consume(RBRACE)
    return result

def return_statement(tokens):
    returnKeyword = tokens.consume(RETURN)
    right = infix_expression(tokens)
    statement_end(tokens)
    return ast.ReturnStatement(returnKeyword, right)

def if_statement(tokens):

    ifKeyword = tokens.consume(IF)
    tokens.consume(LPAREN)
    conditionExpression = infix_expression(tokens)
    tokens.consume(RPAREN)

    tokens.tryConsume(NEWLINE)

    mainBlock = bracketed_statement_list(tokens)
    elseBlock = None

    tokens.tryConsume(NEWLINE)

    if tokens.nextIs(ELSE):
        elseKeyword = tokens.consume(ELSE)
        tokens.tryConsume(NEWLINE)
        elseBlock = bracketed_statement_list(tokens)

    return ast.IfBlock(conditionExpression, mainBlock, elseBlock)

def high_level_option_statement(tokens):
    tokens.consume(LBRACE)
    name = tokens.consume(IDENT)
    tokens.consume(RBRACE)
    statement_end(tokens)
    return ast.HighLevelOptionStatement(name)

def statement_end(tokens):
    """
    Parses the end of a statement. May be indicated by NEWLINE or by the
    end of the stream.
    """
    if tokens.finished():
        return
    else:
        tokens.consume(NEWLINE)


def bracketed_statement_list(tokens):
    """
    Accepts either a series of statements, surrounded by {}s, or a
    single statement.
    Returns a StatementList either way.
    """
    stmtList = ast.StatementList()

    if tokens.nextIs(LBRACKET):
        tokens.consume(LBRACKET)
        while not tokens.nextIs(RBRACKET):
            stmtList.append(statement(tokens))
        tokens.consume(RBRACKET)
    else:
        tokens.tryConsume(NEWLINE)
        stmtList.append(statement(tokens))
    return stmtList

def testEquals():
    import tokens
    ts = tokens.tokenize("a = 1")
    print infix_expression(ts)

if __name__ == '__main__':
    testEquals()

