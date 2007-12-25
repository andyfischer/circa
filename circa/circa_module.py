import builder
import builtin_functions
import term
import token
import utils.indent_printer

from utils.indent_printer import IndentPrinter

class CircaModule(object):
  def __init__(self, environment=None):

    self.global_term = term.create(builtin_functions.SUBROUTINE)
    self.env = environment

    self.file_reference = None
    self.source_tokens = None

    assert self.global_term.state != None

  @classmethod
  def fromText(cls, text):
    module = CircaModule()
    module.source_tokens = token.tokenize(text)
    builder = module.makeBuilder()
    builder.eval(text)
    return module
    

  @classmethod
  def fromFile(cls, file_name):
    mod = CircaModule()
    builder = mod.makeBuilder()
    # todo

  def makeBuilder(self):
    return builder.Builder(self)

  def run(self):
    self.global_term.evaluate()

  def printTerms(self):
    stack = [ self.global_term ]
    printer = IndentPrinter()

    term = stack.pop(0)
    term.printExtended(printer)
    printer.println()

  def getTerm(self, name):
    return self.global_term.__getitem__(name)

  def __getitem__(self, name):
    return self.getTerm(name)

