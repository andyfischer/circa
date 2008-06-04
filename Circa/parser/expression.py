# Expression.py
#
# Code for parsing a Circa expression from a token stream
 

import ast
from token_definitions import *

def parseExpression(token):
    "Parse an expression from the token stream, and return an AST"
    return infix_expression(token, 0)

class MatchFailed(Exception):
    pass

# Infix precedence
HIGHEST_INFIX_PRECEDENCE = 7
_infixPrecedence = {
    DOT: 7,
    STAR: 6, SLASH: 6,
    PLUS: 5, MINUS: 5,
    LTHAN: 2, LTHANEQ: 2, GTHAN: 2, GTHANEQ: 2, DOUBLE_EQUALS: 2, NOT_EQUALS: 2,
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
    raise MatchFailed()
 
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

def testEquals():
    import tokens
    ts = tokens.tokenize("a = 1")
    print infix_expression(ts, 0)

if __name__ == '__main__':
    testEquals()
