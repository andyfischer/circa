
import tokens

class Token(object):
  def __init__(m, match, text, line, column):
    m.match = match
    m.text = text
    m.line = line
    m.column = column

  def length(m):
    return len(m.text)

  def __str__(m):
    return m.text


def tokenize(string):
  currentIndex = 0
  currentLine = 0
  currentCol = 0
  output = []

  def makeToken(token_def, length):
    return Token(token_def, string[currentIndex : currentIndex+length], currentLine, currentCol)

  def testList(token_def_list):
    for token_def in token_def_list:
      match = token_def.pattern.match(string, currentIndex)
      if match:
        return makeToken(token_def, match.end() - match.start())

    return None

  while currentIndex < len(string):

    token = testList(tokens.unkeyed_by_char)

    if not token:
      token = testList(tokens.by_first_char[string[currentIndex]])

    if not token:
      token = makeToken(tokens.UNRECOGNIZED, 1)

    currentIndex += token.length()
    currentCol += token.length()
    output.append(token)

  return output

