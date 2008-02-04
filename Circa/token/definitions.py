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
LEFT_ARROW =    TokenDef(33, 'left_arrow', '<-')
RIGHT_ARROW =   TokenDef(33, 'right_arrow', '->')
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
AND =           TokenDef(40, 'and', 'and')
DEFINE =        TokenDef(41, 'define', 'define')
ELSE =          TokenDef(42, 'else', 'else')
FALSE =         TokenDef(43, 'false', 'false')
FOR =           TokenDef(44, 'for', 'for')
FUNCTION =      TokenDef(45, 'function', 'function')
GLOBAL =        TokenDef(46, 'global', 'global')
IF =            TokenDef(47, 'if', 'if')
NULL =          TokenDef(48, 'null', 'null')
OR =            TokenDef(49, 'or', 'or')
RETURN =        TokenDef(50, 'return', 'return')
STATE =         TokenDef(51, 'state', 'state')
TYPE =          TokenDef(52, 'type', 'type')
TRUE =          TokenDef(53, 'true', 'true')
THIS =          TokenDef(54, 'this', 'this')
VAR =           TokenDef(55, 'var', 'var')

# other types
FLOAT =         TokenDef(70, 'float', pattern=r"([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+)")
INTEGER =       TokenDef(71, 'integer', pattern=r"[1-9]+[0-9]*")
IDENT =         TokenDef(72, 'ident', pattern=r"[a-zA-Z_\-]+[a-zA-Z0-9_\-]*")
STRING =        TokenDef(73, 'string', pattern=group(r"'[^']*'", r"\"[^\"]*\""))
WHITESPACE =    TokenDef(75, 'whitespace', pattern=r"[ \t]+")

# this needs to be below FLOAT
DOT =           TokenDef(27, 'dot', '.')

# meta types
UNRECOGNIZED =  TokenDef(74, 'unrecognized', '')

