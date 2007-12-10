import term, builtin_functions, utils.indent_printer

class CircaModule(object):
  def __init__(self, environment=None):

    self.global_term = term.create(builtin_functions.subroutine)
    self.env = environment

    assert self.global_term.state != None

  def run(self):
    self.global_term.evaluate()

  def printTerms(self):
    stack = [ self.global_term ]
    printer = indent_printer.IndentPrinter()

    while stack:
      term = stack.pop(0)
      term.printExtended(printer)

      # add next terms to stack
      if term.state:
        for branch in term.state.branches:
          for term in branch.terms:
            stack.append(term)


    
