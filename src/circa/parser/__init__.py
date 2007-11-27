from tokens import *
import tokenizer
from circa.module import Module

def parseText(text):
  token_list = tokenizer.tokenize(text)
  print [str(x) for x in token_list]
  parser = Parser(Module(), token_list)
  return parser.module


# Infix precedence
HIGHEST_INFIX_PRECEDENCE = 6
infixPrecedence = {}
for t in [DOT]: infixPrecedence[t] = 6
for t in [STAR, SLASH]: infixPrecedence[t] = 5
for t in [PLUS, MINUS]: infixPrecedence[t] = 4
for t in [LTHAN, LTHANEQ, GTHAN, GTHANEQ, DOUBLE_EQUALS, NOT_EQUALS]:
  infixPrecedence[t] = 3
for t in [AND, OR]: infixPrecedence[t] = 2
for t in [EQUALS, PLUS_EQUALS, MINUS_EQUALS, STAR_EQUALS, SLASH_EQUALS, COLON_EQUALS]:
  infixPrecedence[t] = 1


def getInfixPrecedence(token):
  if token in infixPrecedence:
    return infixPrecedence[token]
  else: return -1

class ParseError(Exception):
  def __init__(m, str, token_location):
    m.str = str
    m.location = token_location

  def __str__(m): return m.str

class Parser(object):
  def __init__(m, module, tokens):
    m.module = module
    m.inputTokens = tokens
    m.tokenIndex = 0
    m.block_stack = []
    m.parse_errors = []
    m.raise_errors = False
    m.previous_block = None
    
    while m.nextToken():
      try:
        m.statement()
      except ParseError, err:
        m.parse_errors.append(err)
        print err
        m.tokenIndex += 1

    

  def nextToken(m, lookahead=0):
    index = m.tokenIndex + lookahead
    if index >= len(m.inputTokens): return None
    return m.inputTokens[index]

  def nextIs(m,match, lookahead=0):
    return m.inputTokens[m.tokenIndex + lookahead].match == match

  def consume(m, match=None):
    token = m.nextToken
    if match and token.match != match:
      raise ParseError("Expected: " + str(match), token)
    m.tokenIndex += 1
    return token

  def bind(m, name, term):
    existing = m.currentBlock().getReference(name)
    m.currentBlock().putReference(name, term)
    if existing:
      m.currentBlock().handleRebind(name, existing, term)


  def statement(m):
    paths = {}
    paths[IF] = m.if_statement
    paths[ELSE] = m.else_statement
    paths[FUNCTION] = m.subroutine_decl
    paths[RETURN] = m.return_statement
    # paths[VAR] = m.var_statement
    # paths[STATE] = m.state_block
    # paths[FOR] = m.for_block
    paths[RBRACE] = m.close_block

    if m.nextToken() in paths:
      paths[m.nextToken()]()
      return

    # default, parse as expression
    expr = m.expression()

    if not expr:
      raise ParseError("Could not understand: " + str(m.nextToken().match), m.nextToken())

  def if_statement(m):
    m.consume(IF)
    m.consume(LPAREN)
    condition_term = expression().eval()
    cond_block = blocks.ConditionBlock(m, condition_term)
    m.consume(RPAREN)

    startBlock(cond_block)

    m.block_body()

  def else_statement(m):
    m.consume(ELSE)

     # todo : reopen previous conditional block

    m.block_body()

  def subroutine_decl(m):
    m.consume(FUNCTION)
    subroutine_name = m.consume(IDENT)
    m.consume(LPAREN)

    # consume arguments
    args = []
    if m.nextIs(IDENT):
      args.append( m.consume(IDENT) )

    while m.nextIs(COMMA):
      args.append( m.consume(IDENT) )

    m.consume(RPAREN)

    subBlock = SubroutineBlock(m, subroutine_name.text)

    subroutine_term = m.module.createConstant(subBlock.subroutine, m.currentBlock())
    m.bind(subroutine_name.text, function_term)

  def return_statement(m):
    return_token = consume(RETURN)
    expr = expression()
    if not expr:
      raise ParseError("Expected expression", return_token)

    bind(Terms.RETURN_REF_NAME, expr.eval())



  def for_block(m):
    m.consume(FOR)
    iterator = m.consume(IDENT)
    m.consume(COLON)
    list_expression = m.expression()
    startBlock(blocks.ForBlock(m, iterator.text, list_expression.eval()))
    m.block_body()

  def close_block(m):
    current_block = m.block_stack[-1]
    current_block.onFinish()
    m.block_stack.pop()
    current_block.afterFinish()
    m.previous_block = current_block


  def expression(m):
    return m.infix_expression(0)

  def infix_expression(m, precedence):
    if (precedence > HIGHEST_INFIX_PRECEDENCE):
      return m.unary_expression()

    expr = m.infix_expression(precedence + 1)
    if not expr: return None

    while getInfixPrecedence(m.nextToken()) == precedence:
      operator = m.consume()

      first_righthand_token = m.nextToken()
      right_expr = m.infix_expression(precedence + 1)

      if not right_expr:
        raise ParseError("Unknown parse error", first_righthand_token)

      expr = InfixNode(m, operator, expr, right_expr)

    return expr

  def unary_expression(m):
    if m.nextIs(MINUS):
      minus = m.consume(MINUS)

      # if next is a number, pass on the minus sign to that number
      if m.nextIs(FLOAT) or m.nextIs(INT):
        m.nextToken().text = "-" + m.nextToken().text
      else:
        return UnaryNode(m, minus, m.atom())

    return m.atom()

  def atom(m):

    # check for a function call
    if m.nextIs(IDENT) and m.nextIs(LPAREN, lookahead=1):
      return m.function_call()

    # check for literal value
    if m.nextToken() in (FLOAT, INTEGER, TRUE, FALSE):
      return LiteralNode(m, m.consume())

    # parenthesized expression
    if m.nextIs(LPAREN):
      m.consume(LPAREN)
      expr = m.expression()
      m.consume(RPAREN)
      return expr
    
