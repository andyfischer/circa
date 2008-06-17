
# Code for turning a string into a list of tokens

import pdb, re
from Circa.common import debug
import parse_errors
from token_definitions import *

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

        # Default is to skip whitespace
        self.skipSet = set([WHITESPACE])

        self.currentIndex = 0

    def next(self, lookahead=0):
        index = self.indexAfterSkipping(self.currentIndex)

        while lookahead > 0:
            index = self.indexAfterSkipping(index + 1)
            lookahead -= 1

        try:
            return self.tokens[index]
        except IndexError:
            return None

    def consume(self, match=None):
        """
        Return the next token and advance our pointer to the next
        non-skip token. Throws an error if a 'match' is specified and
        the next token didn't use that match.
        """

        token = self.next()

        if token is None:
            raise parse_errors.UnexpectedEOF()

        if match and token.match != match:
            raise parse_errors.ExpectedToken(token, match)

        # Advance current index
        self.currentIndex = self.indexAfterSkipping(
                self.indexAfterSkipping(self.currentIndex) + 1)

        return token

    def finished(self):
        return self.currentIndex >= len(self.tokens)

    def reset(self):
        self.currentIndex = 0

    def indexAfterSkipping(self, index):
        """
        Returns the index after skipping. If the token at 'index' is not skippable,
        this function will just return 'index'.

        In the process of skipping, this function may reach the end of the stream
        Callers should probably check for that.
        """

        while True:
            # Finish if we reach the end of the stream
            if index >= len(self.tokens):
                return index

            token = self.tokens[index]

            # Check for a skippable token
            if token.match in self.skipSet:
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

    def stopSkippingEverything(self):
        self.skipSet = set()

    def nextIs(self, match, lookahead=0):
        next = self.next(lookahead)
        if not next: return False
        return next.match == match

    def nextIn(self, match, lookahead=0):
        next = self.next(lookahead)
        if not next: return False
        return self.next(lookahead).match in match

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

    def __str__(self):
        return str(map(str, self.tokens))

    def backToString(self):
        return tokenize.untokenize(self.tokens)
    untokenize = backToString 


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
  
    def makeToken(tokenDef, length):
        return TokenInstance(tokenDef, string[currentIndex : currentIndex+length],
                             currentLine, currentCol)
  
    while currentIndex < len(string):
        token = None
    
        # Find a matching token
        for tdef in ALL_TOKEN_DEFS:
    
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

def tokenStreamTest():
    def fakeToken(tdef):
        return TokenInstance(tdef, "", 0, 0)
    strm = TokenStream(map(fakeToken, [WHITESPACE, IF, WHITESPACE, THIS,
        LPAREN]))

    # Try with whitespace skipping
    strm.consume(IF)
    strm.consume(THIS)
    strm.consume(LPAREN)

    # Now try without whitespace skipping
    strm.reset()
    strm.stopSkippingEverything()
    strm.consume(WHITESPACE)
    strm.consume(IF)
    strm.consume(WHITESPACE)
    strm.consume(THIS)
    strm.consume(LPAREN)

def tokenizeTest():
    strm = tokenize('{x = 1+2}')
    strm.consume('LBRACKET')
    strm.consume('IDENT')
    debug._assert(not strm.finished())
    strm.consume('EQUALS')
    strm.consume('INTEGER')
    strm.consume('PLUS')
    strm.consume('INTEGER')

def commentTest():
    strm = tokenize('{x #= 1+2}')
    strm.consume('LBRACKET')
    strm.consume('IDENT')
    strm.consume('COMMENT_LINE')
    debug._assert(strm.finished())

def commentLineTest():
    strm = tokenize('1 2 3\n# commented line\n  1 # more comments')

def multilineStringTest():
    strm = tokenize('"""hi"""')
    strm.consume('MULTILINE_STR')
    
if __name__ == "__main__":
    tokenStreamTest()
    tokenizeTest()
    commentTest()
    commentLineTest()
    multilineStringTest()

