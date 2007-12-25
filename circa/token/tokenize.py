
import definitions
from token_instance import TokenInstance

def tokenize(string):
  """
  Returns a list of Token instances that fully represents the given string.
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

    # find a matching token
    for tdef in definitions.ALL:

      # skip meta definitions that have no pattern
      if not tdef.pattern: continue

      match = tdef.pattern.match(string, currentIndex)
      if match:
        token = makeToken(tdef, match.end() - match.start())
        break

    # if we didn't find anything, count this character as unrecognized
    if not token:
      token = makeToken(definitions.UNRECOGNIZED, 1)

    # store this token
    currentIndex += token.length()
    currentCol += token.length()
    output_list.append(token)

    # check to advance a line
    if token.match == definitions.NEWLINE:
      currentCol = 1
      currentLine += 1


  return output_list

