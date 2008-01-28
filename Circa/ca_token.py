import re

ALL = []

token_to_name = {}

class TokenDef(object):
  def __init__(self, id, name, raw=None, pattern=None):
    self.id = id
    self.name = name
    self.raw_string = None

    if raw:
      self.pattern = re.compile( re.escape(raw) )
      self.raw_string = raw

    elif pattern:
      self.pattern = re.compile( pattern )

    else:
      self.pattern = None

    ALL.append(self)

  def __str__(self):
    return self.name

  def __eq__(self, other):
    try:
      return self.id == other.id
    except: return False

  def __ne__(self, other):
    try:
      return self.id != other.id
    except: return False

# RE helper functions
def group(*choices): return '(' + '|'.join(choices) + ')'
def any(*choices): return group(*choices) + '*'
def maybe(*choices): return group(*choices) + '?'

# Token definitions

# symbols
LPAREN =        TokenDef(1, 'lparen', '(')
RPAREN =        TokenDef(2, 'rparen', ')')
LBRACE =        TokenDef(3, 'lbrace', '[')
RBRACE =        TokenDef(4, 'rbrace', ']')
LBRACKET =      TokenDef(5, 'lbracket', '{')
RBRACKET =      TokenDef(6, 'rbracket', '}')
PLUS_EQUALS =   TokenDef(7, 'plus_equals', '+=')
PLUS =          TokenDef(8, 'plus', '+')
MINUS_EQUALS =  TokenDef(9, 'minus_equals', '-=')
MINUS =         TokenDef(10, 'minus', '-')
STAR_EQUALS =   TokenDef(11, 'star_equals', '*=')
STAR =          TokenDef(12, 'star', '*')
DOUBLE_SLASH =  TokenDef(13, 'double_slash', '//')
SLASH_EQUALS =  TokenDef(14, 'slash_equals', '/=')
SLASH =         TokenDef(15, 'slash', '/')
LTHANEQ =       TokenDef(16, 'lthaneq', '<=')
LTHAN =         TokenDef(17, 'lthan', '<')
GTHANEQ =       TokenDef(18, 'gthaneq', '>=')
GTHAN =         TokenDef(19, 'gthan', '>')
AT =            TokenDef(20, 'at', '@')
COLON_EQUALS =  TokenDef(21, 'colon_equals', ':=')
COLON =         TokenDef(22, 'colon', ':')
DOUBLE_EQUALS = TokenDef(23, 'double_equals', '==')
NOT_EQUALS =    TokenDef(24, 'not_equals', '!=')
EQUALS =        TokenDef(25, 'equals', '=')
COMMA =         TokenDef(26, 'comma', ',')
QUESTION =      TokenDef(28, 'question', '?')
SEMICOLON =     TokenDef(29, 'semicolon', ';')
POUND =         TokenDef(30, 'pound', '#')
EXCLAIMATION =  TokenDef(31, 'exclaimation', '!')
NEWLINE =       TokenDef(32, 'newline', '\n', pattern=r"\n")

# keywords
FUNCTION =      TokenDef(40, 'function', 'function')
STATE =         TokenDef(41, 'state', 'state')
TYPE =          TokenDef(42, 'type', 'type')
VAR =           TokenDef(43, 'var', 'var')
IF =            TokenDef(44, 'if', 'if')
ELSE =          TokenDef(45, 'else', 'else')
TRUE =          TokenDef(46, 'true', 'true')
FALSE =         TokenDef(47, 'false', 'false')
THIS =          TokenDef(48, 'this', 'this')
GLOBAL =        TokenDef(49, 'global', 'global')
FOR =           TokenDef(50, 'for', 'for')
NULL =          TokenDef(51, 'null', 'null')
RETURN =        TokenDef(53, 'return', 'return')
AND =           TokenDef(55, 'and', 'and')
OR =            TokenDef(56, 'or', 'or')

# other types
FLOAT =         TokenDef(70, 'float', pattern=r"([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+)")
INTEGER =       TokenDef(71, 'integer', pattern=r"[1-9]+[0-9]*")
IDENT =         TokenDef(72, 'ident', pattern=r"[a-zA-Z_\-]+[a-zA-Z0-9_\-]*")
STRING =        TokenDef(73, 'string', pattern=r"'[^']*'")
WHITESPACE =    TokenDef(75, 'whitespace', pattern=r"[ \t]+")

# this needs to be below FLOAT
DOT =           TokenDef(27, 'dot', '.')

# meta types
UNRECOGNIZED =  TokenDef(74, 'unrecognized', '')

class TokenInstance(object):
  """
  An object representing an occurance of a token

  match : the TokenDef object (from definitions) that we matched
  text : the actual source text
  line : the line that this token was located at (starting at 1)
  column : the column that this token starts at (starting at 1)
  """

  def __init__(self, match, text, line, column):
    self.match = match
    self.text = text
    self.line = line
    self.column = column

  def length(self):
    return len(self.text)

  def __str__(self):
    return self.text


def tokenize(string):
  """
  Returns a list of Token instances that fully represents the given string.
  All characters are included, including whitespace and newlines.

  No errors are thrown here, but there may be occurences of the special
  token UNRECOGNIZED.

  For advanced processing of the returned list, consider using a TokenStream.
  """

  currentIndex = 0
  currentLine = 1
  currentCol = 1
  output_list = []

  def makeToken(token_def, length):
    return TokenInstance(token_def, string[currentIndex : currentIndex+length], currentLine, currentCol)

  while currentIndex < len(string):
    token = None

    # Find a matching token
    for tdef in ALL:

      # Skip meta definitions that have no pattern
      if not tdef.pattern: continue

      match = tdef.pattern.match(string, currentIndex)
      if match:
        token = makeToken(tdef, match.end() - match.start())
        break

    # If we didn't find anything, count this character as unrecognized
    if not token:
      token = makeToken(UNRECOGNIZED, 1)

    # Store this token
    currentIndex += token.length()
    currentCol += token.length()
    output_list.append(token)

    # Check to advance a line
    if token.match == NEWLINE:
      currentCol = 1
      currentLine += 1

  return output_list
