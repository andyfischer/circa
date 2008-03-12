from Circa.token.definitions import *

class TypeDecl(object):
   def __init__(self):
      self.id = None
      self.baseType = None
      self.compositeMembers = []
      self.annotations = []

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
   while tokens.nextIs(COLON):
      tokens.consume(COLON)
      decl.annotations.append(tokens.consume(IDENT))

   tokens.stopSkipping(NEWLINE)

   return decl

class FunctionDecl(object):
   def __init__(self):
      self.id = None
      self.args = []
      self.annotations = []
      self.outputType = None

class FunctionArgument(object):
  def __init__(self):
    self.type = None
    self.id = None
    self.outputType = None

  def getNameStr(self):
    if self.id is None: return None
    return self.id.text
      

def function_decl(tokens):
   tokens.consume(FUNCTION)
   decl = FunctionDecl()
   decl.id = tokens.consume(IDENT)
   tokens.consume(LPAREN)

   # Check for input arguments
   if tokens.nextIs(IDENT):
      decl.args.append(function_argument(tokens))
   while tokens.nextIs(COMMA):
      tokens.consume(COMMA)
      decl.args.append(function_argument(tokens))

   # Check for an output
   if tokens.nextIs(RIGHT_ARROW):
      tokens.consume(RIGHT_ARROW)
      decl.outputType = tokens.consume(IDENT)

   tokens.consume(RPAREN)

   # Check for annotations
   while tokens.nextIs(COLON):
      tokens.consume(COLON)
      decl.annotations.append(tokens.consume(IDENT))
      
   return decl

# Returns instance of FunctionArgument
def function_argument(tokens):
   arg = FunctionArgument()
   arg.type = tokens.consume(IDENT)
   if tokens.nextIs(IDENT):
      arg.id = tokens.consume(IDENT)
   return arg

