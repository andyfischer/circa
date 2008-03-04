import pdb

from Circa import (
  ca_function
)

nextGlobalID = 1



class Term(object):
  def __init__(self):
    "Use 'createTerm'. This constructor should be called by almost nobody."

    self.inputs = []
    self.functionTerm = None
    self.users = set()
    self.pythonValue = None
    self.codeUnit = None
    self.branch = None
    self.givenName = None
    self.debugName = None

    global nextGlobalID
    self.globalID = nextGlobalID
    nextGlobalID += 1

  def getType(self):
    "Returns this term's output type"
    # todo, needs to be rewritten to handle multiple outputs
    return self.functionTerm.pythonValue.outputTypes[0]

  def getFunction(self):
    "Returns this term's Function"
    return self.functionTerm.pythonValue

  def getSomeName(self):
    """
    Returns some unique identifier. There are a few values we may use here.
    No guarantees are made as to the format.
    """
    if self.givenName:
        return self.givenName
    elif self.debugName:
        return self.debugName
    else:
        return 't' + str(self.globalID)

  def inputsContain(self, term):
    "Returns True if our inputs contain the term"
    for input in self.inputs:
      if input == term: return True
    return false

  """
  def changeFunction(self, new_func, initial_state=None):
    "Change this term's function"
    self.functionTerm = new_func

    if not initial_state:
      self.state = self.functionTerm.makeState()
    else:
      self.state = initial_state
      """

  def pythonEvaluate(self):
      self.getFunction().pythonEvaluate(self)

  def printExtended(self, printer):
    printer.prints("%i: %s" % (self.globalID, self.functionTerm.pythonValue.name))

    if self.pythonValue:
      printer.prints(" " + str(self.pythonValue))

    printer.prints(" [")

    first_item = True
    for input in self.inputs:
      if not first_item: printer.prints(",")
      printer.prints(input.globalID)
      first_item = False
    printer.prints("]")

    printer.println()

    if self.state:
      label_branches = len(self.state.branches) > 1
      branch_index = -1

      for branch in self.state.branches:
        branch_index += 1

        # (maybe) label the branch index
        if label_branches:
          printer.indent(1)
          printer.println('Branch %i:' % branch_index)
          printer.indent(1)
        else:
          printer.indent(2)

        # print inner terms
        if branch.terms:
          for term in branch.terms:
            term.printExtended(printer)
        else:
          printer.println("(empty)")

        printer.unindent(2)

  def iterate(self, forwards=True):
    yield self
    if self.branch:
      for term in self.branch:
        yield term

  def equals(self, term):
    assert isinstance(term, Term)
    return self.pythonValue == term.pythonValue

  # value accessors
  def __int__(self):
    try: return int(self.pythonValue)
    except: return 0

  def __float__(self):
    try: return float(self.pythonValue)
    except: return 0.0

  def __str__(self):
    try: return str(self.pythonValue)
    except: return ""

  # member accessor
  def __getitem__(self, name):
    return self.state.getLocal(name)



