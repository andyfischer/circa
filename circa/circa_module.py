
import builder
import builtin_functions
import term
import token
from utils.indent_printer import IndentPrinter

class CircaModule(object):
  def __init__(self, environment=None):

    self.global_term = term.Term(builtin_functions.SUBROUTINE)
    self.env = environment

    self.file_reference = None
    self.source_tokens = None

    assert self.global_term.state != None

  @classmethod
  def fromText(cls, text):
    "Create a CircaModule from the given text."

    module = CircaModule()
    module.source_tokens = token.tokenize(text)
    builder = module.makeBuilder()
    builder.eval(module.source_tokens)
    return module
    
  @classmethod
  def fromFile(cls, file_name):
    "Create a CircaModule from the given file."

    # get file contents
    file = open(file_name, 'r')
    file_contents = file.read()
    file.close()
    del file

    # create module using fromText
    module = CircaModule.fromText(file_contents)
    module.file_reference = file_name
    return module

  def makeBuilder(self):
    "Create a new Builder object that manipulates this module"
    return builder.Builder(self)

  def run(self):
    self.global_term.evaluate()

  def printTerms(self):
    "Print the contents of this module, for debugging purposes"
    stack = [ self.global_term ]
    printer = IndentPrinter()

    term = stack.pop(0)
    term.printExtended(printer)
    printer.println()

  def getTerm(self, name):
    return self.global_term[name]

  def __getitem__(self, name):
    return self.getTerm(name)

