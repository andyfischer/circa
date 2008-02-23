
import pdb

from Circa import (
  ca_function,
  signature,
  subroutine,
  terms,
  token
)

from Circa.parser import expression as expression_module
from Circa.token import token_stream
from Circa.token.definitions import *

# Local modules
import parse_errors, blocks

SPECIAL_NAME_FOR_RETURNS = "#return"

VERBOSE_DEBUGGING = False

def parse(builder, source, raise_errors=False):
  parser = Parser(builder, source, raise_errors)
  parser.run()

class Parser(object):
  def __init__(self, builder, token_source, raise_errors):
    self.builder = builder
    self.parse_errors = []
    self.raise_errors = raise_errors

    # make sure 'tokens' is a token stream
    self.tokens = token_stream.asTokenStream(token_source)
    
  def run(self):
    while not self.tokens.finished():
      try:
        self.statement()
      except parse_errors.ParseError, e:
        if self.raise_errors:
          raise

        self.parse_errors.append(e)
        print "[parse error] " + e.description()
        self.tokens.consume()

  def bind(self, name, term):
    existing = self.currentBlock().getReference(name)
    self.currentBlock().putReference(name, term)
    if existing:
      self.currentBlock().handleRebind(name, existing, term)

  def expression(self):
    # Parse an expression using the 'expression' package
    return expression_module.parseExpression(self.tokens)

  def statement(self):
    if VERBOSE_DEBUGGING: print "Parsing statement"

    paths = {}
    paths[IF] = self.if_statement
    paths[FUNCTION] = self.function_decl
    paths[RETURN] = self.return_statement
    paths[RBRACKET] = self.right_bracket
    paths[NEWLINE] = self.new_line

    next_token = self.tokens.next()

    # If we have this token in our paths, parse that
    if next_token.match in paths:
      paths[next_token.match]()
      return

    # otherwise, parse as expression
    expr = self.expression()

    if expr:
      expr.eval(self.builder)
      return
    else:
      if VERBOSE_DEBUGGING: print "parseExpression failed on " + next_token.detailsStr()

    # if we got this far then we don't know what the hell to do
    raise parse_errors.NotAStatement(next_token)

  def if_statement(self):
    if VERBOSE_DEBUGGING: print "Parsing if_statement"

    self.tokens.consume(IF)
    self.tokens.startSkipping(NEWLINE)

    self.tokens.consume(LPAREN)

    # parse the condition expression
    first_condition_token = self.tokens.next()
    condition_expr = self.expression()
    condition_term = condition_expr.eval(self.builder)

    if condition_term == None:
      raise parse_errors.ExpectedExpression(first_condition_token)
    
    self.tokens.consume(RPAREN)

    # create a group & block
    cond_group = self.builder.newConditionalGroup(condition_term)
    cond_block = cond_group.newBlock()
    assert cond_block is not None

    self.block(cond_block)

    if VERBOSE_DEBUGGING:
      print "Finished if block, next token is: " + str(self.tokens.next())

    if self.tokens.nextIs(ELSE):
      if VERBOSE_DEBUGGING: print "Parsing else block"
      self.tokens.startSkipping(NEWLINE)
      self.tokens.consume(ELSE)

      else_block = cond_group.newBlock(isDefault = True)

      self.block(else_block)

    cond_group.finish()

  # Returns instance of Argument
  def function_argument(self):
    arg = Argument()
    arg.type = self.tokens.consume(IDENT)
    if self.tokens.nextIs(IDENT):
      arg.id = self.tokens.consume(IDENT)
    return arg

  def function_decl(self):
    self.tokens.consume(FUNCTION)
    function_id = self.tokens.consume(IDENT)
    self.tokens.consume(LPAREN)

    # collect arguments

    args = []
    if self.tokens.nextIs(IDENT):
      args.append(self.function_argument())

    while self.tokens.nextIs(COMMA):
      self.tokens.consume(COMMA)
      args.append(self.function_argument())

    # check for output type
    outputType = None
    if self.tokens.nextIs(RIGHT_ARROW):
      self.tokens.consume(RIGHT_ARROW)
      self.tokens.outputType = self.tokens.consume(IDENT)

    self.tokens.consume(RPAREN)

    # check for attributes
    attributes = []
    while self.tokens.nextIs(COLON):
      self.tokens.consume(COLON)
      attributes.append( self.tokens.consume(IDENT) )

    arg_names = map(lambda arg: arg.id.text, args)
    attribute_names = map(lambda a: a.text, attributes)

    # Check for 'builtin' attribute
    if 'builtin' in attribute_names:
      # Create a builtin function
      func = ca_function.BaseFunction()
      func.name = function_id.text

      type_arr = map(lambda arg: self.builder.getNamed(arg.type.text), args) 
      func.signature = signature.fixed(type_arr)

      if outputType:
        func.outputType = self.builder.getNamed(outputType.text)

      self.builder.createConstant(value=func, name=func.name)
      return

    # create a new subroutine object
    sub = subroutine.SubroutineDefinition(input_names=arg_names)

    # open a block
    code_block = blocks.CodeUnitBlock(self.builder, sub.code_unit)
    self.block(code_block)

    # store this guy in a constant term
    subroutine_constant = self.builder.createConstant(value=sub, name=function_id.text)


  def return_statement(self):
    return_token = self.tokens.consume(RETURN)
    expr = self.expression()
    if not expr:
      raise parse_errors.ExpectedExpression(return_token)

    self.bind(SPECIAL_NAME_FOR_RETURNS, expr.eval(self.builder))

  def block(self, blockToStart):
    if VERBOSE_DEBUGGING: print "Parsing block"

    if blockToStart:
      self.builder.startBlock(blockToStart)

    # check for a block bounded with {}s
    if (self.tokens.nextIs(LBRACKET)):
      self.tokens.consume(LBRACKET)
      self.tokens.stopSkipping(NEWLINE)

      # parse contents
      while not self.tokens.nextIs(RBRACKET):
        self.statement()

      self.tokens.consume(RBRACKET)

    # parse a one-liner with no {}s
    else:
      self.statement()

    if blockToStart:
      self.builder.closeBlock()

  def right_bracket(self):
    token = self.tokens.consume(RBRACKET)
    if self.builder.blockDepth() < 1:
      raise parse_errors.DanglingRightBracket(token)

    self.builder.closeBlock()

  def new_line(self):
    if VERBOSE_DEBUGGING: print "Parsing new_line"

    self.tokens.consume(NEWLINE)

class Argument(object):
  def __init__(self):
    self.type = None
    self.id = None

