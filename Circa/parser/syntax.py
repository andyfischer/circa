from Circa.token.definitions import *

class TypeDecl(object):
   def __init__(self):
      self.id = None
      self.baseType = None
      self.compositeMembers = []
      self.annotations = []
   def annotationStrings(self):
      return map(lambda a: a.text, self.annotations)

def type_decl(tokens):
   tokens.consume(TYPE)
   tokens.startSkipping(NEWLINE)

   decl = TypeDecl()

   decl.id = tokens.consume(IDENT)

   # Check to parse composite type
   if tokens.nextIs(LBRACKET):

      tokens.consume(LBRACKET)
      while not tokens.nextIs(RBRACKET):
         memberType = tokens.consume(IDENT)
         memberName = tokens.consume(IDENT)
         decl.compositeMembers.append([memberType, memberName])

         if not tokens.nextIs(RBRACKET):
            tokens.consume(COMMA)
      tokens.consume(RBRACKET)

   # Parse a type specialization
   elif tokens.nextIs(EQUALS):
      tokens.consume(EQUALS)
      decl.baseType = tokens.consume(IDENT)

   # Parse annotations
   decl.annotations = annotation_list(tokens)

   tokens.stopSkipping(NEWLINE)

   return decl

class FunctionDecl(object):

   def __init__(self):
      self.id = None
      self.inputArgs = []
      self.annotations = []
      self.outputType = None

   def appendInput(self, type, name):
      class FunctionArgument(object): pass
      input = FunctionArgument()
      input.type = type
      input.name = name
      self.inputArgs.append(input)

   def inputTypes(self):
      return map(lambda a: a.type, self.inputArgs)
   def inputNames(self):
      return map(lambda a: a.name, self.inputArgs)
   def annotationStrings(self):
      return map(lambda a: a.text, self.annotations)

def function_decl(tokens):
   decl = FunctionDecl()
   decl.id = tokens.consume(IDENT)
   tokens.consume(LPAREN)

   # Check for input arguments
   if tokens.nextIs(IDENT):
      decl.appendInput(*function_argument(tokens))
   while tokens.nextIs(COMMA):
      tokens.consume(COMMA)
      decl.appendInput(*function_argument(tokens))

   # Check for an output
   if tokens.nextIs(RIGHT_ARROW):
      tokens.consume(RIGHT_ARROW)
      decl.outputType = tokens.consume(IDENT)

   tokens.consume(RPAREN)

   # Check for annotations
   decl.annotations = annotation_list(tokens)
      
   return decl

# Parses a function argument and returns a pair (type, name).
# 'name' may be None
def function_argument(tokens):
   type = tokens.consume(IDENT)
   id = None
   if tokens.nextIs(IDENT):
      id = tokens.consume(IDENT)
   return (type,id)

# Parse a list of annotations surrounded by []s
# Returns an empty list if none are found
def annotation_list(tokens):
   if not tokens.nextIs(LBRACE):
      return []

   tokens.consume(LBRACE)
   annotationList = []

   while not tokens.nextIs(RBRACE):
      annotationList.append(tokens.consume(IDENT))

   tokens.consume(RBRACE)
   return annotationList
