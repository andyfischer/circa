import builder
import builtin_functions
import term
import utils.indent_printer

from utils.indent_printer import IndentPrinter

class CircaModule(object):
  def __init__(self, environment=None):

    self.global_term = term.create(builtin_functions.SUBROUTINE)
    self.env = environment

    assert self.global_term.state != None

  def startBuilder(self):
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

def fromSource(source):
  module = CircaModule()
  builder = module.startBuilder()
  builder.eval(source)
  return module
