import term, builtin_functions, utils.indent_printer

from utils.indent_printer import IndentPrinter

class CircaModule(object):
  def __init__(self, environment=None):

    self.global_term = term.create(builtin_functions.SUBROUTINE)
    self.env = environment

    assert self.global_term.state != None

  def run(self):
    self.global_term.evaluate()

  def printTerms(self):
    stack = [ self.global_term ]
    printer = IndentPrinter()

    term = stack.pop(0)
    term.printExtended(printer)
    printer.println()


