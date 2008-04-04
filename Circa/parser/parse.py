
import pdb, sys, traceback

from Circa import (
   builtins,
   ca_function,
   code,
   token
)

from Circa.parser import expression as expression_module
from Circa.token import token_stream
from Circa.token.definitions import *

# Local modules
import parse_errors

SPECIAL_NAME_FOR_RETURNS = "#return"

def PLACEHOLDER_EVALUATE_FOR_BUILTINS(term):
   print "Warning: builtin function " + term.functionTerm.getSomeName() + " does not have a body."

def PLACEHOLDER_FUNC_FOR_ATTRIBUTES(term):
   # Do nothing
   pass

VERBOSE_DEBUGGING = False

def parse(builder, source, **parser_options):
   parser = Parser(builder, source, **parser_options)

def parseFile(builder, sourceFile, **parser_options):
   file = open(sourceFile, 'r')
   file_contents = file.read()
   file.close()
   del file
   parser = Parser(builder, file_contents, fileName=sourceFile, **parser_options)

class Parser(object):
   def __init__(self, builder, token_source, raise_errors=False, fileName=None,
         pythonObjectSource=None):
      self.builder = builder
      self.parse_errors = []
      self.raise_errors = raise_errors
      self.fileName = fileName
      self.pythonObjectSource = pythonObjectSource

      # Make sure 'tokens' is a token stream
      self.tokens = token_stream.asTokenStream(token_source)
     
      # Evaluate token source
      while not self.tokens.finished():
         try:
            self.statement()
         except parse_errors.ParseError, e:
            if self.raise_errors:
               raise

            self.parse_errors.append(e)
            print "[parse error] " + str(e)

            # Drop this token, and the rest of the line
            self.tokens.consume()
            self.tokens.dropUntil(NEWLINE)

         except Exception, e:
            if self.raise_errors:
               raise

            print "Internal error on token: " + self.tokens.next().detailsStr()
            print e
            traceback.print_tb(sys.exc_traceback)

   def bind(self, name, term):
      existing = self.currentBlock().getReference(name)
      self.currentBlock().putReference(name, term)
      if existing:
         self.currentBlock().handleRebind(name, existing, term)

   def expression_statement(self):
      # Parse an expression using the 'expression' package
      exprResult = expression_module.parseExpression(self.tokens)

      if exprResult is None:
         raise parse_errors.NotAStatement(self.tokens.next())

      exprResult.eval(self.builder)

   def statement(self):
      if VERBOSE_DEBUGGING: print "Parsing statement"

      paths = {}
      paths[TYPE] = self.type_decl
      paths[IF] = self.if_statement
      paths[FUNCTION] = self.function_decl
      paths[RETURN] = self.return_statement
      paths[RBRACKET] = self.right_bracket
      paths[NEWLINE] = self.new_line
  
      next_token = self.tokens.next()

      # If we have this token in our paths, parse that
      if next_token.match in paths:
         paths[next_token.match]()
         return

      # Check to parse a property
      if self.tokens.nextIs(IDENT) and self.tokens.nextIs(COLON,lookahead=1):
         self.property_statement()
         return

      # Otherwise, parse as expression
      self.expression_statement()

   def type_decl(self):
      decl = type_decl(self.tokens)

      name = decl.id.text

      # Handle a built-in type
      if 'python' in decl.getAnnotationFlagStrings():
         return self.builder.createConstant(value=None, name = name, valueType=builtins.TYPE_TYPE)

      # Handle composite type
      # todo

      # Handle specialize type
      # todo

      raise parse_errors.NotImplemented(name)

   def if_statement(self):
      if VERBOSE_DEBUGGING: print "Parsing if_statement"

      self.tokens.consume(IF)
      self.tokens.startSkipping(NEWLINE)

      self.tokens.consume(LPAREN)

      # parse the condition expression
      first_condition_token = self.tokens.next()
      condition_expr = self.expression()
      condition_term = condition_expr.eval(self.builder)

      if condition_term == None:
         raise parse_errors.ExpectedExpression(first_condition_token)
     
      self.tokens.consume(RPAREN)

      # create a group & block
      cond_group = self.builder.newConditionalGroup(condition_term)
      cond_block = cond_group.newBlock()
      assert cond_block is not None

      self.block(cond_block)

      if VERBOSE_DEBUGGING:
         print "Finished if block, next token is: " + str(self.tokens.next())

      if self.tokens.nextIs(ELSE):
         if VERBOSE_DEBUGGING: print "Parsing else block"
         self.tokens.startSkipping(NEWLINE)
         self.tokens.consume(ELSE)

         else_block = cond_group.newBlock(isDefault = True)

         self.block(else_block)

      cond_group.finish()

   def function_decl(self):
      self.tokens.consume(FUNCTION)
      decl = function_decl(self.tokens)

      def getArgName(arg): return arg.getNameStr()

      # Get a list of input types
      inputTypes = map(self.getTypeFromToken, decl.getInputTypes())
      outputType = (None if decl.outputType is None
                         else self.getTypeFromToken(decl.outputType))

      name = decl.id.text
      funcObj = None

      # Check for 'python' annotation
      if 'python' in decl.getAnnotationFlagStrings():

         if self.pythonObjectSource is None:
            raise parse_errors.NoPythonSourceProvided(decl.findAnnotation('python'))

         if name not in self.pythonObjectSource:
            raise parse_errors.PythonObjectNotFound(decl.id)

         # Fetch the python-based function
         funcObj = self.pythonObjectSource[name]

         # Overwrite type information with declared types
         funcObj.inputTypes = inputTypes
         funcObj.outputType = outputType

      else:

         raise parse_errors.NotImplemented(tokens.next())

         # Create a new subroutine object
         sub = code.SubroutineDefinition(input_names=decl.getInputNames())

         # Open a block
         code_block = builder.FunctionBlock(self.builder, sub.code_unit)
         self.block(code_block)

      # Check for 'training' annotation


      # Store result in a constant term
      return self.builder.createConstant(value=funcObj, name=name)

   """
   def attribute_decl(self):
      self.tokens.consume(ATTRIBUTE)
      decl = syntax.function_decl(self.tokens)

      inputTypes = map(self.getTypeFromToken, decl.inputTypes())
      outputType = self.getTypeFromToken(decl.outputType)

      funcObject = ca_function.Function(inputTypes, outputType)
      funcObject.name = decl.id.text
      funcObject.pythonEvaluate = PLACEHOLDER_FUNC_FOR_ATTRIBUTES
      self.builder.createConstant(value=funcObject, name=decl.id.text)
   """

   def return_statement(self):
      return_token = self.tokens.consume(RETURN)
      expr = self.expression()
      if not expr:
         raise parse_errors.ExpectedExpression(return_token)

      self.bind(SPECIAL_NAME_FOR_RETURNS, expr.eval(self.builder))

   def block(self, blockToStart):
      if VERBOSE_DEBUGGING: print "Parsing block"

      if blockToStart:
        self.builder.startBlock(blockToStart)

      # check for a block bounded with {}s
      if (self.tokens.nextIs(LBRACKET)):
        self.tokens.consume(LBRACKET)
        self.tokens.stopSkipping(NEWLINE)

        # parse contents
        while not self.tokens.nextIs(RBRACKET):
          self.statement()

        self.tokens.consume(RBRACKET)

      # parse a one-liner with no {}s
      else:
        self.statement()

      if blockToStart:
        self.builder.closeBlock()

   def right_bracket(self):
      token = self.tokens.consume(RBRACKET)
      if self.builder.blockDepth() < 1:
        raise parse_errors.DanglingRightBracket(token)

      self.builder.closeBlock()

   def property_statement(self):
      property = self.tokens.consume(IDENT)
      self.tokens.consume(COLON)

      # Handle specific property
      if property.text == "namespace":
         # Parse namespace
         namespace = self.tokens.consume(IDENT)

         # TODO: accept namespaces of the form blah.whatever.something

         self.builder.startNamespace(namespace.text)

      else:
         raise parse_errors.UnrecognizedProperty(property)

   def new_line(self):
      if VERBOSE_DEBUGGING: print "Parsing new_line"
      self.tokens.consume(NEWLINE)

   def getTypeFromToken(self, token):

      typeTerm = self.builder.getNamed(token.text)

      if typeTerm is None:
         raise parse_errors.IdentifierNotFound(token)

      if typeTerm.getType() is not builtins.TYPE_TYPE:
         raise parse_errors.IdentifierIsNotAType(token)

      return typeTerm
     
class ParsedFunctionDecl(object):
   def getAnnotationFlagStrings(self):
      return map(lambda a: a.text, self.annotations.flags)

   def getInputTypes(self):
      return map(lambda a: a.type, self.inputArgs)

   def getInputNames(self):
      return map(lambda a: a.name, self.inputArgs)

class ParsedFunctionArg(object):
   pass

def function_decl(tokens):
   """
   Parse a function declaration, and returns an object of type
   ParsedFunctionDecl. This object contains:
     .id = the function's name (token)
     .inputArgs = a list of arguments. Each object in this list contains:
        .type = the type name (token)
        .name = the identifier (token). May be None
     .annotations = a list of annotations (tokens)
     .outputType = the return type name (token). May be None
   """
   result = ParsedFunctionDecl()
   result.id = tokens.consume(IDENT)
   tokens.consume(LPAREN)

   # Check for input arguments
   result.inputArgs = []
   if tokens.nextIs(IDENT):
      result.inputArgs.append(function_argument(tokens))
   while tokens.nextIs(COMMA):
      tokens.consume(COMMA)
      result.inputArgs.append(function_argument(tokens))

   result.outputType = None
   if tokens.nextIs(RIGHT_ARROW):
      tokens.consume(RIGHT_ARROW)
      result.outputType = tokens.consume(IDENT)

   tokens.consume(RPAREN)

   # Check for annotations
   result.annotations = annotation_list(tokens)
      
   return result

def function_argument(tokens):
   """
   Parses a function argument and returns an object of type ParsedFunctionArg.
   This object contains:
      .name = the argument's name (token). May be None
      .type = the argument's type (token)
   """
   result = ParsedFunctionArg()
   result.type = tokens.consume(IDENT)
   result.name = None
   if tokens.nextIs(IDENT):
      result.name = tokens.consume(IDENT)
   return result

class ParsedTypeDecl(object):
   def getAnnotationFlagStrings(self):
      return map(lambda a: a.text, self.annotations.flags)

def type_decl(tokens):
   """
   Parses a type declaration and returns an object of type ParsedTypeDecl.
   This object contains:
      .id = the type's name (token)
      .annotations = a list of annotations
   """

   result = ParsedTypeDecl()

   tokens.consume(TYPE)
   tokens.startSkipping(NEWLINE)

   result.id = tokens.consume(IDENT)

   # Check to parse composite type
   if tokens.nextIs(LBRACKET):

      result.compositeMembers = []

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
      result.baseType = tokens.consume(IDENT)

   # Parse annotations
   result.annotations = annotation_list(tokens)

   tokens.stopSkipping(NEWLINE)

   return result

class AnnotationList(object):
   def __init__(self):
      self.flags = []
      self.associative = {}

# Parse a list of annotations surrounded by []s
# Returns an empty list if none are found
def annotation_list(tokens):
   if not tokens.nextIs(LBRACE):
      return None

   tokens.consume(LBRACE)
   annotationList = AnnotationList()

   while not tokens.nextIs(RBRACE):
      annotationName = tokens.consume(IDENT)
      
      if tokens.nextIs(EQUALS):
         tokens.consume(EQUALS)
         annotationList.associative[annotationName] = tokens.consume(IDENT)
      else:
         annotationList.flags.append(annotationName)

      if tokens.nextIs(COMMA):
         tokens.consume(COMMA)

   tokens.consume(RBRACE)
   return annotationList


