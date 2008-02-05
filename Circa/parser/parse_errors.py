
class ParseError(Exception):
  def __init__(self, location):
    self.location = location

  def __str__(self):
    return "(line " + str(self.location.line) + ":" + \
        str(self.location.column) + ") " + self.message()

  def message(self):
    pass

class TokenStreamExpected(ParseError):
  def __init__(self, expected, location):
    ParseError.__init__(self, location)
    self.expected = expected

  def message(self):
    return "Expected: " + self.expected.text

class IdentifierNotFound(ParseError):
  def __init__(self, identifier_token):
    ParseError.__init__(self, identifier_token)
    self.identifier_token = identifier_token
  def message(self):
    return "Identifier not found: " + self.identifier_token.text

class DanglingRightBracket(ParseError):
  def message(self):
    return "Found } without a corresponding {"

class NotAStatement(ParseError):
  def message(self):
    return "Not a statement"

class ExpectedExpression(ParseError):
  def message(self):
    return "Expected a valid expression"

class InternalError(ParseError):
  def message(self):
    return "Internal error"


