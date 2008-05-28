
# Code for turning a string into a list of tokens

import re

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

class TokenInstance(object):
    """
    An object representing an occurance of a token
  
    match : the TokenDef object (from definitions) that we matched
    text : the actual source text
    line : the line where this token was located (starting at 1)
    column : the column where this token was located (starting at 1)
    """
  
    def __init__(self, match, text, line, column):
        self.match = match
        self.text = text
        self.line = line
        self.column = column
  
    def length(self):
        return len(self.text)
  
    def name(self):
        return self.match.name
  
    def __str__(self):
        return self.text
  
    def detailsStr(self):
        text = self.text
    
        # for certain characters, don't display them literally
        if text == "\n":
            text = "<newline>"
    
        return "(%i:%i %s,%s)" % (self.line, self.column, text, self.match)

class TokenStream(object):
    def __init__(self, tokens):
        """
        Create a TokenStream from a list of tokens.
        """

        assert isinstance(tokens, list)

        self.tokens = tokens

        # Default is to skip whitespace (change this)
        self.skipSet = set([WHITESPACE])

        # currentIndex should always rest on a non-skip token,
        # so advance past any skip tokens that are at the start
        self.currentIndex = self.indexAfterSkipping(0)

    def __str__(self):
        return str(map(str, self.tokens))

    def finished(self):
        return self.currentIndex >= len(self.tokens)

    def reset(self):
        self.currentIndex = 0

    def indexAfterSkipping(self, index):
        """
        Returns the index after skipping. If the token at 'index' is not skippable,
        this function will just return 'index'.

        There are currently two cases that we skip: 1) individual tokens that are
        in skipSet (which includes WHITESPACE and sometimes NEWLINE), and 2) when we
        encounter a POUND we skip up to NEWLINE

        In the process of skipping, this function may reach the end of the stream
        Callers should probably check for that.
        """

        currentlyInComment = False

        while True:
            # Finish if we reach the end of the stream
            if index >= len(self.tokens):
                return index

            token = self.tokens[index]

            # Check to start a comment
            if isCommentStart(token):
                currentlyInComment = True

            # Check to finish comment
            elif token.match is NEWLINE:
                currentlyInComment = False

            # Check for a skippable token
            if token.match in self.skipSet:
                pass

            # Continue a comment
            elif currentlyInComment:
                pass

            # Otherwise finish and return
            else:
                return index

            # Iterate
            index += 1

    def shouldSkip(self, token):
        return token.match in self.skipSet

    def startSkipping(self, token_def):
        self.skipSet.add(token_def)

    def stopSkipping(self, token_def):
        if token_def in self.skipSet:
            self.skipSet.remove(token_def)

    def next(self, lookahead=0):
        index = self.currentIndex

        while lookahead > 0:
            index = self.indexAfterSkipping(index + 1)
            lookahead -= 1

        try:
            return self.tokens[index]
        except IndexError:
            return None

    def nextIs(self, match, lookahead=0):
        next = self.next(lookahead)
        if not next: return False
        return next.match == match

    def nextIn(self, match, lookahead=0):
        next = self.next(lookahead)
        if not next: return False
        return self.next(lookahead).match in match

    def consume(self, match=None):
        """
        Return the next token and advance our pointer to the next
        non-skip token. Throws an error if a 'match' is specified and
        the next token didn't use that match.
        """

        token = self.next()
        if match and token.match != match:
          raise parse_errors.TokenStreamExpected(match, token)

        # advance current index
        self.currentIndex = self.indexAfterSkipping(self.currentIndex + 1)

        return token

    def dropUntil(self, match):
        """
        Drop all tokens until we find a token with 'match'. We also drop the
        matching token.
        """
        while True:
            token = self.consume()
            if token is None: break
            if self.finished(): break
            if token.match == match: break

    def markLocation(self):
        """
        Returns a mark object that represents the current location,
        suitable for a call to restoreLocation
        """
        return self.currentIndex

    def restoreMark(self, mark):
        self.currentIndex = mark

    def backToString(self):
        return tokenize.untokenize(self.tokens)
    untokenize = backToString 


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
IDENT =         TokenDef(72, 'ident', pattern=r"[a-zA-Z_\-][a-zA-Z0-9_\-]*")
STRING =        TokenDef(73, 'string', pattern=group(r"'[^']*'", r"\"[^\"]*\""))
WHITESPACE =    TokenDef(75, 'whitespace', pattern=r"[ \t]+")
#POUND_IDENT =   TokenDef(75, 'pound_ident', pattern="#" + word(alphanumeric+'*'))

# this needs to be below FLOAT
DOT =           TokenDef(27, 'dot', '.')

# this needs to be below POUND_IDENT
POUND =         TokenDef(30, 'pound', '#')

# Meta types
UNRECOGNIZED =  TokenDef(80, 'unrecognized', '')

def tokenize(string):
    """
    Tokenize the given string, and return a TokenStream object.
  
    No errors are thrown here, but there may be occurences of the special
    token UNRECOGNIZED, and these should probably be treated as errors.
    """
  
    currentIndex = 0
    currentLine = 1
    currentCol = 1
    output_list = []
  
    def makeToken(token_def, length):
        return TokenInstance(token_def, string[currentIndex : currentIndex+length],
                             currentLine, currentCol)
  
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

    # Return this list wrapped in a TokenStream
    return TokenStream(output_list)

def untokenize(token_list):
    """
    Returns a string created from the given list of TokenInstances.
    """

    return "".join(map(lambda t: t.text, token_list))

def run_test():
    tokens = tokenize("ifblah")
    assert tokens[0].match == IDENT

    tokens = tokenize("if ")
    assert tokens[0].match == IF

    tokens = tokenize("function ")
    assert tokens[0].match == FUNCTION

if __name__ == "__main__":
    run_test()

