import token_definitions
from circa.parser import ParseError
import types

def defaultSkipFunction(token):
  return token.match == token_definitions.WHITESPACE

class TokenStream(object):
  def __init__(self, tokens, skipFunction=defaultSkipFunction):
    assert isinstance(tokens, list)
    assert isinstance(skipFunction, types.FunctionType)

    self.tokens = tokens
    self.currentIndex = 0
    self.skip = skipFunction

    # currentIndex should always rest on a non-skip token,
    # so advance past any skip tokens that are at the start
    while not self.finished() and self.skip(self.tokens[self.currentIndex]):
      self.currentIndex += 1

  def __str__(self):
    return str(map(str, self.tokens))

  def finished(self):
    return self.currentIndex >= len(self.tokens)

  def reset(self):
    self.currentIndex = 0

  def advance(self, index):
    index += 1
    while index < len(self.tokens) and self.skip(self.tokens[index]):
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
    token = self.next()
    if match and token.match != match:
      raise ParseError("Expected: " + str(match), token)

    # advance current index
    self.currentIndex = self.advance(self.currentIndex)

    return token
    
    




