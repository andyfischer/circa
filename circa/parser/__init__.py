
class ParseError(Exception):
  def __init__(self, str, token_location):
    self.str = str
    self.location = token_location

  def __str__(self):
    return self.str


from token_definitions import *
from expression import parseExpression
import token, expression
from token_stream import TokenStream
from circa.circa_module import CircaModule
from circa.builder import Builder
import unittest

def parse(text, *args, **kwargs):
  tokens = token.toTokenStream(text)
  parser = Parser(CircaModule(), tokens)
  parser.parse(*args, **kwargs)
  return parser.module

parseText = parse


class Parser(object):
  def __init__(self, module, tokens):
    self.module = module
    self.builder = Builder(module)
    self.tokens = tokens
    self.block_stack = []
    self.parse_errors = []
    self.raise_errors = False
    self.previous_block = None

  def parse(self, throwErrors=False):
    while not self.tokens.finished():
      try:
        self.statement()
      except ParseError, err:

        if throwErrors:
          raise

        self.parse_errors.append(err)
        print "[parse error] " + str(err)
        self.tokens.consume()

  def bind(self, name, term):
    existing = self.currentBlock().getReference(name)
    self.currentBlock().putReference(name, term)
    if existing:
      self.currentBlock().handleRebind(name, existing, term)


  def statement(self):
    paths = {}
    paths[IF] = self.if_statement
    paths[ELSE] = self.else_statement
    paths[FUNCTION] = self.subroutine_decl
    paths[RETURN] = self.return_statement
    # paths[VAR] = self.var_statement
    # paths[STATE] = self.state_block
    # paths[FOR] = self.for_block
    paths[RBRACE] = self.close_block

    if self.tokens.next() in paths:
      paths[self.tokens.next()]()
      return

    # default, parse as expression
    expr = parseExpression(self.tokens)

    if not expr:
      raise ParseError("Could not understand: " + str(self.tokens.next().text), self.tokens.next())

    expr.eval(self.builder)

  def if_statement(self):
    self.tokens.consume(IF)
    self.tokens.consume(LPAREN)
    condition_term = parseExpression(tokens).eval()
    cond_block = blocks.ConditionBlock(self, condition_term)
    self.tokens.consume(RPAREN)

    startBlock(cond_block)

    self.block_body()

  def else_statement(self):
    self.tokens.consume(ELSE)

     # todo : reopen previous conditional block

    self.block_body()

  def subroutine_decl(self):
    self.tokens.consume(FUNCTION)
    subroutine_name = self.tokens.consume(IDENT)
    self.tokens.consume(LPAREN)

    # tokens.consume arguments
    args = []
    if self.tokens.nextIs(IDENT):
      args.append( self.tokens.consume(IDENT) )

    while self.tokens.nextIs(COMMA):
      args.append( self.tokens.consume(IDENT) )

    self.tokens.consume(RPAREN)

    subBlock = SubroutineBlock(self, subroutine_name.text)

    subroutine_term = self.builder.createConstant(subBlock.subroutine, self.currentBlock())
    self.bind(subroutine_name.text, function_term)

  def return_statement(self):
    return_token = tokens.consume(RETURN)
    expr = self.expression()
    if not expr:
      raise ParseError("Expected expression", return_token)

    self.bind(Terms.RETURN_REF_NAME, expr.eval())



  def for_block(self):
    self.tokens.consume(FOR)
    iterator = self.tokens.consume(IDENT)
    self.tokens.consume(COLON)
    list_expression = self.expression()
    startBlock(blocks.ForBlock(self, iterator.text, list_expression.eval()))
    self.block_body()

  def close_block(self):
    current_block = self.block_stack[-1]
    current_block.onFinish()
    self.block_stack.pop()
    current_block.afterFinish()
    self.previous_block = current_block



# Tests
class Tests(unittest.TestCase):
  def testSimple(self):
    module = parse("a = 1 + 2", throwErrors=True)


if __name__ == '__main__':
    unittest.main()
