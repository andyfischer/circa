import re

ALL = []

token_to_name = {}

by_first_char = {}
unkeyed_by_char = []


class TokenDef(object):
  def __init__(self, id, name, raw=None, pattern=None):
    self.id = id
    self.name = name
    self.raw_string = None

    if raw:
      self.pattern = re.compile( re.escape(raw) )
      self.raw_string = raw

      # add to by_first_char map
      global by_first_char
      if not raw[0] in by_first_char:
        by_first_char[raw[0]] = []
      by_first_char[raw[0]].append(self)

    elif pattern:

      self.pattern = re.compile( pattern )

      global unkeyed_by_char
      unkeyed_by_char.append(self)

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
      


# symbols
LPAREN = TokenDef(1, 'lparen', '(')
RPAREN = TokenDef(2, 'rparen', ')')
LBRACE = TokenDef(3, 'lbrace', '[')
RBRACE = TokenDef(4, 'rbrace', ']')
LBRACKET = TokenDef(5, 'lbracket', '{')
RBRACKET = TokenDef(6, 'rbracket', '}')
PLUS_EQUALS = TokenDef(7, 'plus_equals', '+=')
PLUS = TokenDef(8, 'plus', '+')
MINUS_EQUALS = TokenDef(9, 'minus_equals', '-=')
MINUS = TokenDef(10, 'minus', '-')
STAR_EQUALS = TokenDef(11, 'star_equals', '*=')
STAR = TokenDef(12, 'star', '*')
DOUBLE_SLASH = TokenDef(13, 'double_slash', '//')
SLASH_EQUALS = TokenDef(14, 'slash_equals', '/=')
SLASH = TokenDef(15, 'slash', '/')
LTHANEQ = TokenDef(16, 'lthaneq', '<=')
LTHAN = TokenDef(17, 'lthan', '<')
GTHANEQ = TokenDef(18, 'gthaneq', '>=')
GTHAN = TokenDef(19, 'gthan', '>')
AT = TokenDef(20, 'at', '@')
COLON_EQUALS = TokenDef(21, 'colon_equals', ':=')
COLON = TokenDef(22, 'colon', ':')
DOUBLE_EQUALS = TokenDef(23, 'double_equals', '==')
NOT_EQUALS = TokenDef(24, 'not_equals', '!=')
EQUALS = TokenDef(25, 'equals', '=')
COMMA = TokenDef(26, 'comma', ',')
DOT = TokenDef(27, 'dot', '.')
QUESTION = TokenDef(28, 'question', '?')
SEMICOLON = TokenDef(29, 'semicolon', ';')
POUND = TokenDef(30, 'pound', '#')
EXCLAIMATION = TokenDef(31, 'exclaimation', '!')
NEWLINE = TokenDef(32, 'newline', '\n', pattern=r"\n")

# keywords
FUNCTION = TokenDef(40, 'function', 'function')
STATE = TokenDef(41, 'state', 'state')
TYPE = TokenDef(42, 'type', 'type')
VAR = TokenDef(43, 'var', 'var')
IF = TokenDef(44, 'if', 'if')
ELSE = TokenDef(45, 'else', 'else')
TRUE = TokenDef(46, 'true', 'true')
FALSE = TokenDef(47, 'false', 'false')
THIS = TokenDef(48, 'this', 'this')
GLOBAL = TokenDef(49, 'global', 'global')
FOR = TokenDef(50, 'for', 'for')
NULL = TokenDef(51, 'null', 'null')
RETURN = TokenDef(53, 'return', 'return')
AND = TokenDef(55, 'and', 'and')
OR = TokenDef(56, 'or', 'or')

# other types
FLOAT = TokenDef(70, 'float', pattern=r"[0-9]*\.[0-9]*")
INTEGER = TokenDef(71, 'integer', pattern=r"[1-9]+[0-9]*")
IDENT = TokenDef(72, 'ident', pattern=r"[a-zA-Z_\-]+[a-zA-Z0-9_\-]*")
WHITESPACE = TokenDef(75, 'whitespace', pattern=r"[ \t]+")

# meta types
UNRECOGNIZED = TokenDef(74, 'unrecognized', '')
