
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
import parse_errors, syntax

SPECIAL_NAME_FOR_RETURNS = "#return"

def PLACEHOLDER_EVALUATE_FOR_BUILTINS(term):
   print "Warning: builtin function " + term.functionTerm.getSomeName() + " does not have a body."

def PLACEHOLDER_FUNC_FOR_ATTRIBUTES(term):
   # Do nothing
   pass

VERBOSE_DEBUGGING = False

def parse(builder, source, **parser_options):
   parser = Parser(builder, source, **parser_options)
   parser.run()

def parseFile(builder, sourceFile, raise_errors=False):
   file = open(sourceFile, 'r')
   file_contents = file.read()
   file.close()
   del file
   parse(builder, file_contents, raise_errors=raise_errors, file_name=sourceFile)

class Parser(object):
  def __init__(self, builder, token_source, raise_errors=False, file_name=None):
     self.builder = builder
     self.parse_errors = []
     self.raise_errors = raise_errors
     self.file_name = file_name

     # Make sure 'tokens' is a token stream
     self.tokens = token_stream.asTokenStream(token_source)
    
  def run(self):
     while not self.tokens.finished():
        try:
           self.statement()
        except parse_errors.ParseError, e:
           if self.raise_errors:
              raise

           self.parse_errors.append(e)
           print "[parse error] " + e.description()

           # Drop this token, and the rest of the line
           self.tokens.consume()
           self.tokens.dropUntil(NEWLINE)

        except Exception, e:
           if self.raise_errors:
              raise

           print "Internal error on token: " + self.tokens.next().detailsStr()
           print e
           traceback.print_tb(sys.exc_traceback)

  def raiseError(self, errorDetails):
     raise parse_errors.ParseError(errorDetails, file_name=self.file_name)

  def bind(self, name, term):
     existing = self.currentBlock().getReference(name)
     self.currentBlock().putReference(name, term)
     if existing:
        self.currentBlock().handleRebind(name, existing, term)

  def expression_statement(self):
     # Parse an expression using the 'expression' package
     exprResult = expression_module.parseExpression(self.tokens)

     if exprResult is None:
        self.raiseError(parse_errors.NotAStatement(self.tokens.next()))

     exprResult.eval(self.builder)

  def statement(self):
     if VERBOSE_DEBUGGING: print "Parsing statement"

     paths = {}
     paths[TYPE] = self.type_decl
     paths[IF] = self.if_statement
     paths[ATTRIBUTE] = self.attribute_decl
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
     decl = syntax.type_decl(self.tokens)

     name = decl.id.text

     # Handle a built-in type
     if "builtin" in decl.annotationStrings():
        return self.builder.createConstant(value=None, name = name, constType=builtins.TYPE_TYPE)

     # Handle composite type
     # todo

     # Handle specialize type
     # todo

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
       raiseError(parse_errors.ExpectedExpression(first_condition_token))
    
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
     decl = syntax.function_decl(self.tokens)

     def getArgName(arg): return arg.getNameStr()

     # Get a list of input types
     inputTypes = map(self.getTypeFromToken, decl.inputTypes())
     outputType = (None if decl.outputType is None
                        else self.getTypeFromToken(decl.outputType))

     # Check for 'builtin' annotation
     if 'builtin' in decl.annotationStrings():

        # Create a builtin function
        func = ca_function.Function(inputTypes, outputType)
        func.name = decl.id.text
        func.pythonEvaluate = PLACEHOLDER_EVALUATE_FOR_BUILTINS

        return self.builder.createConstant(value=func, name=decl.id.text)

     # Create a new subroutine object
     sub = code.SubroutineDefinition(input_names=decl.inputNames())

     # open a block
     code_block = builder.FunctionBlock(self.builder, sub.code_unit)
     self.block(code_block)

     # store this guy in a constant term
     return self.builder.createConstant(value=sub, name=decl.id.text)

  def attribute_decl(self):
     self.tokens.consume(ATTRIBUTE)
     decl = syntax.function_decl(self.tokens)

     inputTypes = map(self.getTypeFromToken, decl.inputTypes())
     outputType = self.getTypeFromToken(decl.outputType)

     funcObject = ca_function.Function(inputTypes, outputType)
     funcObject.name = decl.id.text
     funcObject.pythonEvaluate = PLACEHOLDER_FUNC_FOR_ATTRIBUTES
     self.builder.createConstant(value=funcObject, name=decl.id.text)

  def return_statement(self):
     return_token = self.tokens.consume(RETURN)
     expr = self.expression()
     if not expr:
        raiseError(parse_errors.ExpectedExpression(return_token))

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
      raiseError(parse_errors.DanglingRightBracket(token))

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
        raiseError(parse_errors.UnrecognizedProperty(property))

  def new_line(self):
    if VERBOSE_DEBUGGING: print "Parsing new_line"

    self.tokens.consume(NEWLINE)


  def getTypeFromToken(self, token):

     typeTerm = self.builder.getNamed(token.text)

     if typeTerm is None:
        raiseError(parse_errors.IdentifierNotFound(token))

     if typeTerm.getType() is not builtins.TYPE_TYPE:
        raiseError(parse_errors.IdentifierIsNotAType(token))

     return typeTerm
      


