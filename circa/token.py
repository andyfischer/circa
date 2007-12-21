
import token_definitions

class Token(object):
  """
  An object representing a matched token.

  match : the TokenDef object (from token_definitions) that we matched as
  text : the actual source text
  line : the line that this token was located at (starting at 1)
  column : the column that this token starts at (starting at 1)
  """

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
  """
  Returns a flat list of Token instances that represents the given string.

  For advanced processing of a token list, consider using a TokenStream.
  """

  currentIndex = 0
  currentLine = 1
  currentCol = 1
  output_list = []

  def makeToken(token_def, length):
    return Token(token_def, string[currentIndex : currentIndex+length], currentLine, currentCol)

  while currentIndex < len(string):
    token = None

    # find a matching token
    for tdef in token_definitions.ALL:
      match = tdef.pattern.match(string, currentIndex)
      if match:
        token = makeToken(tdef, match.end() - match.start())
        break

    # if we didn't find anything, count this character as unrecognized
    if not token:
      token = makeToken(token_definitions.UNRECOGNIZED, 1)

    # store this token
    currentIndex += token.length()
    currentCol += token.length()
    output_list.append(token)

    # check to advance a line
    if token.match == token_definitions.NEWLINE:
      currentCol = 1
      currentLine += 1


  return output_list
