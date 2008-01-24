
import pdb

import expression
import parse_errors
import token
from token.definitions import *
import terms

def parse(builder, source):
  tokens = token.asTokenStream(source)
  parser = Parser(builder, tokens, raise_errors=True)
  parser.run()

class Parser(object):
  def __init__(self, builder, token_source, raise_errors=True):
    self.builder = builder
    self.parse_errors = []
    self.raise_errors = raise_errors
    self.previous_block = None

    # make sure 'tokens' is a token stream
    self.tokens = token.asTokenStream(token_source)
    
  def run(self):
    while not self.tokens.finished():
      try:
        self.statement()
      except parse_errors.ParseError, e:
        if self.raise_errors:
          raise

        self.parse_errors.append(e)
        print "[parse error] " + str(e)
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
    paths[RBRACKET] = self.right_bracket
    paths[NEWLINE] = self.new_line

    next_token = self.tokens.next()

    if next_token.match in paths:
      paths[next_token.match]()
      return

    # otherwise, parse as expression
    expr = expression.parseExpression(self.tokens)
    if expr:
      expr.eval(self.builder)
      return

    # if we got this far then we don't know what the hell to do
    pdb.set_trace()
    raise parse_errors.NotAStatement(next_token)

  def if_statement(self):
    self.tokens.consume(IF)
    self.tokens.startSkipping(NEWLINE)

    self.tokens.consume(LPAREN)

    # parse the condition expression
    first_condition_token = self.tokens.next()
    condition_expr = expression.parseExpression(self.tokens)
    condition_term = condition_expr.eval(self.builder)

    if condition_term == None:
      raise parse_errors.ExpectedExpression(first_condition_token)
    
    self.tokens.consume(RPAREN)

    cond_block = self.builder.startConditionalBlock(condition=condition_term)

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
      raise parse_errors.ExpectedExpression(return_token)

    self.bind(terms.Term.RETURN_REF_NAME, expr.eval())

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
      self.tokens.stopSkipping(NEWLINE)

      # when the } comes up, it will trigger closeBlock
    else:
      # parse a one-liner with no {}s
      self.statement()
      self.builder.closeBlock()

  def right_bracket(self):
    token = self.tokens.consume(RBRACKET)
    if self.builder.blockDepth() < 1:
      raise parse_errors.DanglingRightBracket(token)

    self.builder.closeBlock()

  def new_line(self):
    self.tokens.consume(NEWLINE)

  

