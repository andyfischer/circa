import pdb
import builtin_function_defs
import parse_errors
import terms
import token
from token.definitions import *
from token import Token

DEBUG_LEVEL = 0

# Expression parsing
def parseExpression(tokens):
  # Coerce 'tokens' into a token stream
  tokens = token.asTokenStream(tokens)

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
      return builder.createTerm(function, inputs=[self.left.eval(builder), self.right.eval(builder)] )

    # evaluate as an assignment?
    if self.token.match == EQUALS:
      right_term = self.right.eval(builder)
      if not isinstance(right_term, terms.Term):
        pdb.set_trace()
        raise ParseError("Expression did not evaluate to a term: " + str(self.right), self.getFirstToken())
      return builder.bind(self.left.getName(), right_term)

    # evaluate as a function + assign?
    if self.token.match in infix_token_to_assign_function:
      pass # todo

    raise "Unable to evaluate token: " + self.token.text

  def getFirstToken(self):
    return self.left.getFirstToken()

  def __str__(self):
    return self.function.text + "(" + str(self.left) + "," + str(self.right) + ")"



# Infix token-to-function map
infix_token_to_function = {
    PLUS: builtin_function_defs.ADD,
    MINUS: builtin_function_defs.SUB,
    STAR: builtin_function_defs.MULT,
    SLASH: builtin_function_defs.DIV
}

# Infix token-to-stateful-function
infix_token_to_assign_function = {
    PLUS_EQUALS: builtin_function_defs.ADD,
    MINUS_EQUALS: builtin_function_defs.SUB,
    STAR_EQUALS: builtin_function_defs.MULT,
    SLASH_EQUALS: builtin_function_defs.DIV
}


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
    return builder.createTerm(builtin_function_defs.MULT,
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
      raise parse_errors.InteralError(first_righthand_token)

    expr = Infix(operator, expr, right_expr)

  return expr

def unary_expression(tokens):
  if tokens.nextIs(MINUS):
    minus = tokens.consume(MINUS)
    return Unary(minus, atom(tokens))
  else:
    return atom(tokens)

def atom(tokens):

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
 
  raise MatchFailed()
 
def function_call(tokens):
  function_name = tokens.consume(IDENT)
  tokens.consume(LPAREN)

  args = []
  args.append( infix_expression(tokens, 0) )

  while tokens.nextIs(COMMA):
    tokens.consume(COMMA)
    args.append( infix_expression(tokens, 0) )

  tokens.consume(RPAREN)

  return Function(function_name, args)
 
def parseStringLiteral(text):
  # the literal should have ' marks on either side, strip these
  return text.strip("'\"")
