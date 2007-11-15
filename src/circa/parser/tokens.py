
all = []

token_to_name = {}



def token(id, name, raw_string):
  all.append(id)
  token_to_name[id] = name
  return id


# symbols
LPAREN = token(1, 'lparen', '(')
RPAREN = token(2, 'rparen', ')')
LBRACE = token(3, 'lbrace', '[')
RBRACE = token(4, 'rbrace', ']')
LBRACKET = token(5, 'lbracket', '{')
RBRACKET = token(6, 'rbracket', '}')
PLUS_EQUALS = token(7, 'plus_equals', '+=')
PLUS = token(8, 'plus', '+')
MINUS_EQUALS = token(9, 'minus_equals', '-=')
MINUS = 10
STAR_EQUALS = 11
STAR = 12
DOUBLE_SLASH = 13
SLASH_EQUALS = 14
SLASH = 15
LTHANEQ = 16
LTHAN = 17
GTHANEQ = 18
GTHAN = 19
AT = 20
COLON_EQUALS = 21
COLON = 22
DOUBLE_EQUALS = 23
NOT_EQUALS = 24
EQUALS = 25
COMMA = 26
DOT = 27
QUESTION = 28
SEMICOLON = 29
POUND = 30
EXCLAIMATION = 31

# keywords
FUNCTION = 40
STATE = 41
TYPE = 42
VAR = 43
IF = 44
ELSE = 45
TRUE = 46
FALSE = 47
THIS = 48
GLOBAL = 49
FOR = 50
NULL = 51
EVENT = 52
RETURN = 53
ON = 54
AND = 55
OR = 56

# other types
INTEGER = 70
FLOAT = 71
IDENT = 72
NEWLINE = 73
UNRECOGNIZED = 74
WHITESPACE = 75
COMMENT = 76
