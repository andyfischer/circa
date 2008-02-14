import pdb

from Circa import (
  builtin_functions,
  terms,
  token,
)

from Circa.token.definitions import *
from Circa.token import token_stream

import parse_errors

VERBOSE_DEBUGGING = False

# Expression parsing
def parseExpression(tokens):
  # Coerce 'tokens' into a token stream
  tokens = token_stream.asTokenStream(tokens)

  # Mark the start location for backtracking
  start_loc = tokens.markLocation()

  try:
    return infix_expression(tokens, 0)
  except MatchFailed, e:
    # Backtrack if we didn't match
    tokens.restoreMark(start_loc)
    return None

# Alias for parseExpression
parse = parseExpression

# AST Classes
class Node(object):
  def eval(self, builder):
    raise Exception("Need to implement this")

  def getFirstToken(self):
    raise Exception("Need to implement this")

class Infix(Node):
  def __init__(self, function_token, left, right):
    assert isinstance(function_token, token.Token)
    assert isinstance(left, Node)
    assert isinstance(right, Node)

    self.token = function_token
    self.left = left
    self.right = right

  def eval(self, builder):

    # evaluate as a function?
    if self.token.match in infix_token_to_function:
      function = infix_token_to_function[self.token.match]
      return builder.createTerm(function,
          inputs=[self.left.eval(builder), self.right.eval(builder)] )

    # evaluate as an assignment?
    if self.token.match == EQUALS:
      right_term = self.right.eval(builder)
      if not isinstance(right_term, terms.Term):
        raise ParseError("Expression did not evaluate to a term: " + str(self.right), self.getFirstToken())
      return builder.bind(self.left.getName(), right_term)

    # evaluate as a function + assign?
    if self.token.match in infix_token_to_assign_function:

      # create a term that's the result of the operation
      function = infix_token_to_assign_function[self.token.match]
      result_term = builder.createTerm(function,
          inputs=[self.left.eval(builder), self.right.eval(builder)])

      # bind the name to this result
      return builder.bind(self.left.getName(), result_term)

    # evaluate as stateful assign?
    if self.token.match is COLON_EQUALS:


    raise "Unable to evaluate token: " + self.token.text

  def getFirstToken(self):
    return self.left.getFirstToken()

  def __str__(self):
    return self.function.text + "(" + str(self.left) + "," + str(self.right) + ")"


# Infix token-to-function map
infix_token_to_function = {
    PLUS: builtin_functions.ADD,
    MINUS: builtin_functions.SUB,
    STAR: builtin_functions.MULT,
    SLASH: builtin_functions.DIV,
    DOUBLE_EQUALS: builtin_functions.EQUAL,
    NOT_EQUALS: builtin_functions.NOT_EQUAL
}



# Infix precedence
HIGHEST_INFIX_PRECEDENCE = 6
infixPrecedence = {
    DOT: 6,
    STAR: 5, SLASH: 5,
    PLUS: 4, MINUS: 4,
    LTHAN: 3, LTHANEQ: 3, GTHAN: 3, GTHANEQ: 3, DOUBLE_EQUALS: 3, NOT_EQUALS: 3,
    AND: 2, OR: 2,
    EQUALS: 1, PLUS_EQUALS: 1, MINUS_EQUALS: 1, STAR_EQUALS: 1, SLASH_EQUALS: 1,
      COLON_EQUALS: 1
}

def getInfixPrecedence(token):
  if token and token.match in infixPrecedence:
    return infixPrecedence[token.match]
  else: return -1
  

class Literal(Node):
  def __init__(self, token):
    self.token = token

    if token.match == FLOAT:
      self.value = float(token.text)
    elif token.match == INTEGER:
      self.value = int(token.text)
    elif token.match == STRING:
      self.value = parseStringLiteral(token.text)
    elif token.match == TRUE:
      self.value = True
    elif token.match == FALSE:
      self.value = False
    else:
      raise "Couldn't recognize token: " + str(token)

  def eval(self, builder):
    return builder.createConstant(self.value, source_token=self.token)

  def getFirstToken(self):
    return self.token

  def __str__(self):
    return str(self.value)

class Ident(Node):
  def __init__(self, token):
    self.token = token

  def eval(self, builder):
    term = builder.getNamed(self.token.text)

    if not term:
      raise parse_errors.IdentifierNotFound(self.token)

    return builder.getNamed(self.token.text)

  def getFirstToken(self):
    return self.token

  def getName(self):
    return self.token.text

  def __str__(self):
    return self.token.text

class Unary(Node):
  def __init__(self, function_token, right):
    self.function_token = function_token
    self.right = right

  def eval(self, builder):
    return builder.createTerm(builtin_functions.MULT,
                              inputs = [builder.createConstant(-1),
                                        self.right.eval(builder)])

  def getFirstToken(self):
    return self.function_token;

  def __str__(self):
    return self.function_token.text + "(" + str(self.right) + ")"

class Function(Node):
  def __init__(self, function_name, args):
    self.function_name = function_name
    self.args = args

  def eval(self, builder):
    arg_terms = [t.eval(builder) for t in self.args]
    func = builder.getLocalFunction(self.function_name.text)

    if func is None:
      raise parse_errors.InternalError(self.function_name,
          "Function " + self.function_name.text + " not found.")

    return builder.createTerm(func, inputs=arg_terms)

  def getFirstToken(self):
    return self.function_name;

class MatchFailed(Exception):
  pass


def infix_expression(tokens, precedence):
  if (precedence > HIGHEST_INFIX_PRECEDENCE):
    return unary_expression(tokens)

  expr = infix_expression(tokens, precedence + 1)
  if not expr: return None

  while getInfixPrecedence(tokens.next()) == precedence:
    operator = tokens.consume()

    first_righthand_token = tokens.next()
    right_expr = infix_expression(tokens, precedence + 1)

    if not right_expr:
      raise parse_errors.InternalError(first_righthand_token)

    expr = Infix(operator, expr, right_expr)

  return expr

def unary_expression(tokens):
  if tokens.nextIs(MINUS):
    minus = tokens.consume(MINUS)
    return Unary(minus, atom(tokens))
  else:
    return atom(tokens)

def atom(tokens):

  if VERBOSE_DEBUGGING:
    print "atom, next = " + tokens.next().name()

  # function call
  if tokens.nextIs(IDENT) and tokens.nextIs(LPAREN, lookahead=1):
    return function_call(tokens)

  # literal
  if tokens.nextIn((FLOAT, INTEGER, STRING, TRUE, FALSE)):
    token = tokens.consume()
    return Literal(token)

  # identifier
  if tokens.nextIs(IDENT):
    token = tokens.consume()
    return Ident(token)

  # parenthesized expression
  if tokens.nextIs(LPAREN):
    tokens.consume(LPAREN)
    expr = infix_expression(tokens, 0)
    tokens.consume(RPAREN)
    return expr
 
  # failed to match
  if VERBOSE_DEBUGGING:
    print "atom failed to match"

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

  return Function(function_name, args)
 
def parseStringLiteral(text):
  # the literal should have ' marks on either side, strip these
  return text.strip("'\"")
