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
      if token.match is POUND:
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
