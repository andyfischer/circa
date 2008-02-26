
from definitions import *


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


def tokenize(string):
  """
  Returns a list of TokenInstances that fully represents the given string.
  All characters are included, including whitespace and newlines.

  No errors are thrown here, but there may be occurences of the special
  token UNRECOGNIZED.

  For advanced processing of the returned list, consider using a TokenStream.
  """

  currentIndex = 0
  currentLine = 1
  currentCol = 1
  output_list = []

  def makeToken(token_def, length):
    return TokenInstance(token_def, string[currentIndex : currentIndex+length], currentLine, currentCol)

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

  return output_list

def untokenize(token_list):
  """
  Returns a string created from the given list of TokenInstances.
  """

  return "".join(map(lambda t: t.text, token_list))
