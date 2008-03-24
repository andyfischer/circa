import pdb

class ParseError(Exception):
   def __init__(self, errorDetails, file_name):
      self.location = errorDetails[0]
      self.message = errorDetails[1]

      self.description = "(line " + str(self.location.line) + ":" + \
         str(self.location.column) + ") " + self.message

      if file_name is not None:
         self.description = "File: " + file_name + ", " + self.description

   def __str__(self):
      return self.description

def TokenStreamExpected(expected, location):
   return (location, "Expected: " + expected.name + ", found: " + location.text)

def IdentifierNotFound(identifier):
   return (identifier, "Identifier not found: " + identifier.text)

def IdentifierIsNotAType(ident):
   return (ident, "Not a valid type: " + ident.text)

def DanglingRightBracket(token):
   return (token, "Found } without a corresponding {")

def NotAStatement(token):
   return (token, "Not a statement: " + token.detailsStr())

def ExpectedExpression(token):
   return (token, "Expected a valid expression")

def UnrecognizedProperty(property):
   return (property, "Unrecognized property: " + property.text)

def InternalError(token, details=None):
   message = "Internal error"
   if details is not None: message += ": " + details
   return (token, message)


