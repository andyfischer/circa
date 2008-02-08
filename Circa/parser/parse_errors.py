
class ParseError(Exception):
  def __init__(self, location, message=None):
    self.location = location
    self.message = message

  def __str__(self):
    return "(line " + str(self.location.line) + ":" + \
        str(self.location.column) + ") " + self.message

class TokenStreamExpected(ParseError):
  def __init__(self, expected, location):
    message = "Expected: " + expected.text
    ParseError.__init__(self, location, message)

class IdentifierNotFound(ParseError):
  def __init__(self, identifier_token):
    message = "Identifier not found: " + identifier_token.text
    ParseError.__init__(self, identifier_token, message)

class DanglingRightBracket(ParseError):
  def __init__(self, token):
    ParseError.__init__(self, identifier_token, "Found } without a corresponding {")

class NotAStatement(ParseError):
  def __init__(self, token):
    ParseError.__init__(self, identifier_token, "Not a statement")

class ExpectedExpression(ParseError):
  def __init__(self, token):
    ParseError.__init__(self, identifier_token, "Expected a valid expression")

class InternalError(ParseError):
  def __init__(self, token, details=None):
    message = "Internal error"
    if details: message += ": " + details
    ParseError.__init__(self, token, message)


