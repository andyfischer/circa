import tokenize
from Circa.parser import parse_errors
from definitions import *

def asTokenStream(source):
  """
  Convert 'source' into a TokenStream. Accepts a string, a list
  of tokens, or an existing TokenStream.

  If 'source' is already a TokenStream, this method returns the
  original object without modification.
  """

  if isinstance(source, TokenStream):
    return source

  if isinstance(source, str):
    source = tokenize.tokenize(source)
  
  return TokenStream(source)


class TokenStream(object):
  def __init__(self, tokens):
    """
    Create a TokenStream from a list of tokens.
    """

    assert isinstance(tokens, list)

    self.tokens = tokens
    self.currentIndex = 0
    self.skipSet = set([WHITESPACE])

    # currentIndex should always rest on a non-skip token,
    # so advance past any skip tokens that are at the start
    while not self.finished() and self.shouldSkip(self.tokens[self.currentIndex]):
      self.currentIndex += 1

  def __str__(self):
    return str(map(str, self.tokens))

  def finished(self):
    return self.currentIndex >= len(self.tokens)

  def reset(self):
    self.currentIndex = 0

  def shouldSkip(self, token):
    return token.match in self.skipSet

  def startSkipping(self, token_def):
    self.skipSet.add(token_def)

  def stopSkipping(self, token_def):
    self.skipSet.remove(token_def)

  def advance(self, index):
    index += 1
    while index < len(self.tokens) and self.shouldSkip(self.tokens[index]):
      index += 1
    return index

  def next(self, lookahead=0):
    index = self.currentIndex

    while lookahead > 0:
      index = self.advance(index)
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
    self.currentIndex = self.advance(self.currentIndex)

    return token

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
