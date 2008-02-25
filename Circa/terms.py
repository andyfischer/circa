import pdb

from Circa import (
  ca_function
)

nextGlobalID = 1

def createTerm(functionTerm, initial_value=None, code_unit=None, source_token=None):

  assert isinstance(functionTerm, Term) or functionTerm is None

  term = Term()

  # initialize
  term.inputs = []
  term.functionTerm = functionTerm
  term.source_token = source_token
  term.users = set()
  term.pythonValue = initial_value
  term.codeUnit = code_unit
  term.branch = None

  if functionTerm is not None:
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

    for user_of_input in input.users:
      # Check if they are using the same function
      if user_of_input.functionTerm != functionTerm: continue

      # Check if all the inputs are the same
      def matches(pair):
        return pair[0].equals(pair[1])

      inputs_match = all(map(matches, zip(inputs, user_of_input.inputs)))

      # Todo: allow for functions that don't care what the function order is

      if not inputs_match: continue

      # Looks like this term is the same as what they want
      return user_of_input

  return None
 
  

class Term(object):

  def getType(self):
    "Returns this term's output type"
    return self.functionTerm.pythonValue.outputType

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
    self.functionTerm.pythonValue.pythonEvaluate(self)

  def getIdentifier(self):
    return str(self.globalID)

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



