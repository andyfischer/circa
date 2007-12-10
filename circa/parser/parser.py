
from token_definitions import *
import token, expression
from token_stream import TokenStream
from circa.circa_module import CircaModule
from circa.builder import Builder

DEBUG_LEVEL = 4

def parseText(text):
  tokens = token.token_stream(text)
  parser = Parser(CircaModule(), tokens)
  return parser.module


class ParseError(Exception):
  def __init__(m, str, token_location):
    m.str = str
    m.location = token_location

  def __str__(m): return m.str

class Parser(object):
  def __init__(m, module, tokens):
    m.module = module
    m.builder = Builder(module)
    m.tokens = tokens
    m.block_stack = []
    m.parse_errors = []
    m.raise_errors = False
    m.previous_block = None
    
    while not m.tokens.finished():
      try:
        m.statement()
      except ParseError, err:
        m.parse_errors.append(err)
        print "[parse error] " + str(err)
        m.tokens.consume()

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

    if m.tokens.next() in paths:
      paths[m.tokens.next()]()
      return

    # default, parse as expression
    expr = m.expression()

    if not expr:
      raise ParseError("Could not understand: " + str(m.tokens.next().match), m.tokens.next())

  def if_statement(m):
    m.tokens.consume(IF)
    m.tokens.consume(LPAREN)
    condition_term = m.expression().eval()
    cond_block = blocks.ConditionBlock(m, condition_term)
    m.tokens.consume(RPAREN)

    startBlock(cond_block)

    m.block_body()

  def else_statement(m):
    m.tokens.consume(ELSE)

     # todo : reopen previous conditional block

    m.block_body()

  def subroutine_decl(m):
    m.tokens.consume(FUNCTION)
    subroutine_name = m.tokens.consume(IDENT)
    m.tokens.consume(LPAREN)

    # tokens.consume arguments
    args = []
    if m.tokens.nextIs(IDENT):
      args.append( m.tokens.consume(IDENT) )

    while m.tokens.nextIs(COMMA):
      args.append( m.tokens.consume(IDENT) )

    m.tokens.consume(RPAREN)

    subBlock = SubroutineBlock(m, subroutine_name.text)

    subroutine_term = m.builder.createConstant(subBlock.subroutine, m.currentBlock())
    m.bind(subroutine_name.text, function_term)

  def return_statement(m):
    return_token = tokens.consume(RETURN)
    expr = m.expression()
    if not expr:
      raise ParseError("Expected expression", return_token)

    m.bind(Terms.RETURN_REF_NAME, expr.eval())



  def for_block(m):
    m.tokens.consume(FOR)
    iterator = m.tokens.consume(IDENT)
    m.tokens.consume(COLON)
    list_expression = m.expression()
    startBlock(blocks.ForBlock(m, iterator.text, list_expression.eval()))
    m.block_body()

  def close_block(m):
    current_block = m.block_stack[-1]
    current_block.onFinish()
    m.block_stack.pop()
    current_block.afterFinish()
    m.previous_block = current_block



  

