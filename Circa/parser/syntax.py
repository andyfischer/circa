from Circa.token.definitions import *


class CompositeTypeDecl:
   class Member:
      def __init__(self, type, name):
         self.type = type
         self.name = name

   def __init__(self, name):
      self.name = name
      self.members = []

class SpecializeTypeDelc:
   def __init__(self, name, baseType):
      self.name = name
      self.baseType = baseType

def type_decl(tokens):
   tokens.consume(TYPE)
   tokens.startSkipping(NEWLINE)

   typeName = tokens.consume(IDENT)

   parsedDecl = None

   # Check to parse composite type
   if tokens.nextIs(LBRACKET):
      parsedDecl = CompositeTypeDecl(typeName)

      tokens.consume(LBRACKET)
      while not tokens.nextIs(RBRACKET):
         memberType = tokens.consume(IDENT)
         memberName = tokens.consume(IDENT)
         parsedDecl.members.append(CompositeTypeDecl.Member(memberType, memberName))

         if not tokens.nextIs(RBRACKET):
            tokens.consume(COMMA)
      tokens.consume(RBRACKET)

   # Parse a type specialization
   else:
      tokens.consume(EQUALS)
      baseType = tokens.consume(IDENT)

      parsedDecl = SpecializeTypeDelc(typeName, baseType)

   tokens.stopSkipping(NEWLINE)

   return parsedDecl

