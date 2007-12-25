

class TokenInstance(object):
  """
  An object representing an occurance of a token

  match : the TokenDef object (from definitions) that we matched as
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

