
class ParseError(Exception):
  def __init__(self, text, token_location):
    self.text = str(text)
    self.location = token_location

  def fullDescription(self):
    return "(line " + str(self.location.line) + ", col " + \
        str(self.location.column) + ") " + self.text

  def __str__(self): return self.text


