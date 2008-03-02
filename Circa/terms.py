import pdb

from Circa import (
  ca_function
)

nextGlobalID = 2

def createTerm(functionTerm, initial_value=None, code_unit=None, source_token=None):

  assert isinstance(functionTerm, Term)

  term = Term()

  # initialize
  term.functionTerm = functionTerm
  term.source_token = source_token
  term.pythonValue = initial_value
  term.codeUnit = code_unit

  #term.state = functionTerm.pythonValue.makeState()

  # possibly create a branch
  if functionTerm.pythonValue.hasBranch:
      term.branch = []

  # assign a global ID
  global nextGlobalID
  term.globalID = nextGlobalID
  nextGlobalID += 1

  return term

def findExisting(functionTerm, inputs=[]):
  """
  This function finds an existing term that uses the given function, and has the given
  inputs. Returns None if none found.
  """
  if inputs is None:
    return None

  assert isinstance(functionTerm, Term)
  assert isinstance(functionTerm.pythonValue, ca_function.Function)

  function = functionTerm.pythonValue

  # Try to find an existing term
  for input in inputs:
    assert input is not None

    for potentialMatch in input.users:
      # Check if they are using the same function
      if potentialMatch.functionTerm != functionTerm: continue

      # Check if all the inputs are the same
      def matches(pair):
        return pair[0].equals(pair[1])

      inputs_match = all(map(matches, zip(inputs, potentialMatch.inputs)))

      # Todo: allow for functions that don't care what the function order is

      if not inputs_match: continue

      # Looks like this term is the same as what they want
      return potentialMatch

  return None

def findExistingConstant(constantFunction, value):
    """
    This helper term attempts to find a constant with the given constant-func.
    """

    for possibleMatch in constantFunction.users:
        
        if possibleMatch.functionTerm != constantFunction:
            continue

        if possibleMatch.pythonValue == value:
            return possibleMatch

    return None

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



