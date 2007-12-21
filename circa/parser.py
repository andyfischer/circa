
import pdb

from token_definitions import *
import token, expression
from term import Term
import token_stream
from token_stream import TokenStream
from circa_module import CircaModule
from parse_error import ParseError

DEBUG_LEVEL = 4

def parse(builder, source):
  tokens = token_stream.asTokenStream(source)
  parser = Parser(builder, tokens)
  parser.run()

class Parser(object):
  def __init__(self, builder, tokens):
    self.builder = builder
    self.block_stack = []
    self.parse_errors = []
    self.raise_errors = False
    self.previous_block = None

    # convert 'tokens' into a TokenStream
    if not isinstance(tokens, TokenStream):
      tokens = TokenStream(tokens)

    self.tokens = tokens
    
  def run(self):
    while not self.tokens.finished():
      try:
        self.statement()
      except ParseError, e:
        self.parse_errors.append(e)
        print "[parse error] " + e.fullDescription()
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
    paths[NEWLINE] = self.new_line

    next_token = self.tokens.next()

    if next_token.match in paths:
      paths[next_token.match]()
      return

    # default, parse as expression
    expr = expression.parseExpression(self.tokens)

    if not expr:
      #pdb.set_trace()
      raise ParseError("Couldn't understand (as statement): " + str(next_token.match) + "(" + next_token.match.name + ")", next_token)

  def if_statement(self):
    self.tokens.consume(IF)
    self.tokens.startSkipping(NEWLINE)

    self.tokens.consume(LPAREN)

    # parse the condition expression
    first_condition_token = self.tokens.next()
    condition_expr = expression.parseExpression(self.tokens)
    condition_term = condition_expr.eval(self.builder)

    if condition_term == None:
      raise ParseError("Expected expression", first_condition_token)
    
    cond_block = self.builder.startConditionalBlock(condition=condition_term)
    self.tokens.consume(RPAREN)

    self.builder.startBlock(cond_block)

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

    self.bind(Term.RETURN_REF_NAME, expr.eval())

  def for_block(self):
    self.tokens.consume(FOR)
    iterator = self.tokens.consume(IDENT)
    self.tokens.consume(COLON)
    list_expression = self.expression()
    startBlock(blocks.ForBlock(self, iterator.text, list_expression.eval()))
    self.block_body()

  def block_body(self):

    # check for a block bounded with {}s
    if (self.tokens.nextIs(LBRACKET)):
      self.tokens.consume(LBRACKET)
      self.stopSkipping(NEWLINE)

      # when the } comes up, it will trigger closeBlock
    else:
      # parse a one-liner with no {}s
      self.statement()
      self.closeBlock()

  def close_block(self):
    current_block = self.block_stack[-1]
    current_block.onFinish()
    self.block_stack.pop()
    current_block.afterFinish()
    self.previous_block = current_block

  def new_line(self):
    self.tokens.consume(NEWLINE)

  

