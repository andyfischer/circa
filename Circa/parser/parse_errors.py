import pdb

class ParseError(Exception):
   def __init__(self, location, message):
      self.location = location
      self.message = message
      self.fileName = None

   def __str__(self):
      description = "(line " + str(self.location.line) + ":" + \
         str(self.location.column) + ") " + self.message

      if self.fileName is not None:
         description = "in file " + self.fileName + ", " + description

      return description

def TokenStreamExpected(expected, location):
   return ParseError(location, "Expected: " + expected.name + ", found: " + location.text)

def IdentifierNotFound(identifier):
   return ParseError(identifier, "Identifier not found: " + identifier.text)

def IdentifierIsNotAType(ident):
   return ParseError(ident, "Not a valid type: " + ident.text)

def DanglingRightBracket(token):
   return ParseError(token, "Found } without a corresponding {")

def NotAStatement(token):
   return ParseError(token, "Not a statement: " + token.detailsStr())

def NotImplemented(token):
   return ParseError(token, "Feature is not implemented yet")

def ExpectedExpression(token):
   return ParseError(token, "Expected a valid expression")

def UnrecognizedProperty(property):
   return ParseError(property, "Unrecognized property: " + property.text)

def CouldntFindTrainingFunction(token):
   return ParseError(token, "Couldn't find a training function: " + token.text)

def NoPythonSourceProvided(token):
   return ParseError(token, "No python-object source was provided")

def PythonObjectNotFound(token):
   return ParseError(token, "Python object was not found: " + token.text)

def CantUseImplantOperatorOnLiteral(token):
   return ParseError(token, "Can't have a literal on the left side of the := operator")

def ExpressionDidNotEvaluateToATerm(token):
   return ParseError(token, "Expression did not evaluate to a term")

def InternalError(token, details=None):
   message = "Internal error"
   if details is not None: message += ": " + details
   return ParseError(token, message)
