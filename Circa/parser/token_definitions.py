
import pdb, re

ALL_TOKEN_DEFS = []

class TokenDef(object):
    """
    Defines a type of Token.
    Includes a regular expression for matching this token.
    """
    def __init__(self, id, name, raw=None, pattern=None):
        self.id = id
        self.name = name
        self.raw_string = None

        if raw:
            self.pattern = re.compile( re.escape(raw) )
            self.raw_string = raw

        elif pattern:
            self.pattern = re.compile( pattern, re.MULTILINE )

        else:
            self.pattern = None

        ALL_TOKEN_DEFS.append(self)
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


# Regular expression helper functions
def group(*choices): return '(' + '|'.join(choices) + ')'
def any(*choices): return group(*choices) + '*'
def maybe(*choices): return group(*choices) + '?'
alpha = r"[a-zA-Z_\-]"
alphanumeric = r"[a-zA-Z0-9_\-]"
def word(pattern): return pattern + "(?!" + alphanumeric + ")"

### Token definitions ###

# Symbols
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
EXCLAIMATION =  TokenDef(31, 'exclaimation', '!')
NEWLINE =       TokenDef(32, 'newline', '\n', pattern=r"\n")

# Keywords
DEFINE =        TokenDef(41, 'define', pattern=word('define'))
ELSE =          TokenDef(42, 'else', pattern=word('else'))
FOR =           TokenDef(44, 'for', pattern=word('for'))
FUNCTION =      TokenDef(45, 'function', pattern=word('function'))
GLOBAL =        TokenDef(46, 'global', pattern=word('global'))
IF =            TokenDef(47, 'if', pattern=word('if'))
NULL =          TokenDef(48, 'null', pattern=word('null'))
RETURN =        TokenDef(50, 'return', pattern=word('return'))
STATE =         TokenDef(51, 'state', pattern=word('state'))
TYPE =          TokenDef(52, 'type', pattern=word('type'))
THIS =          TokenDef(54, 'this', pattern=word('this'))
PATCH =         TokenDef(55, 'patch', pattern=word('patch'))
ATTRIBUTE =     TokenDef(56, 'attribute', pattern=word('attribute'))

# Other tokens
FLOAT =         TokenDef(70, 'float', pattern=r"([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+)")
INTEGER =       TokenDef(71, 'integer', pattern=r"([1-9]+[0-9]*)|0")
IDENT =         TokenDef(72, 'identifier', pattern=r"[a-zA-Z_\-][a-zA-Z0-9_\-]*")
STRING =        TokenDef(73, 'string', pattern=group(r"'[^']*'", r"\"[^\"]*\""))
WHITESPACE =    TokenDef(75, 'whitespace', pattern=r"[ \t]+")
COMMENT_LINE =  TokenDef(76, 'comment_line', pattern=r"#.*$")

# this needs to be below FLOAT
DOT =           TokenDef(27, 'dot', '.')

# Meta types
UNRECOGNIZED =  TokenDef(80, 'unrecognized', '')
