
class ParseError(Exception):
  def __init__(self, text, token_location):
    self.text = str(text)
    self.location = token_location

  def fullDescription(self):
    return str(self)

  def __str__(self):
    return "(line " + str(self.location.line) + ":" + \
        str(self.location.column) + ") " + self.text


