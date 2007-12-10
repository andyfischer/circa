
import token_definitions
from token_stream import TokenStream

class Token(object):
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
  currentIndex = 0
  currentLine = 0
  currentCol = 0
  output_list = []

  def makeToken(token_def, length):
    return Token(token_def, string[currentIndex : currentIndex+length], currentLine, currentCol)

  def testList(token_def_list):
    for token_def in token_def_list:
      match = token_def.pattern.match(string, currentIndex)
      if match:
        return makeToken(token_def, match.end() - match.start())

    return None

  while currentIndex < len(string):

    token = testList(token_definitions.unkeyed_by_char)

    if not token:
      token = testList(token_definitions.by_first_char[string[currentIndex]])

    if not token:
      token = makeToken(token_definitions.UNRECOGNIZED, 1)

    currentIndex += token.length()
    currentCol += token.length()
    output_list.append(token)

  return output_list

def toTokenStream(string):
  return TokenStream( tokenize(string) )
