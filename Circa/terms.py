import pdb

from Circa import (
  ca_function
)

nextGlobalID = 1


class Term(object):
  def __init__(self, functionTerm, initial_value=None, code_unit=None, source_token=None):

    assert isinstance(functionTerm, Term) or functionTerm is None

    # initialize
    self.inputs = []
    self.functionTerm = functionTerm
    self.source_token = source_token
    self.users = set()
    self.pythonValue = initial_value
    self.codeUnit = code_unit
    self.state = None
    self.branch = None

    if functionTerm is not None:
      self.state = functionTerm.value.makeState()

      # possibly create a branch
      if functionTerm.value.hasBranch:
        self.branch = []

    """
    # possibly make training info
    if functionTerm.trainingType:
      self.training_info = functionTerm.trainingType()
      self.training_info.update(self)
    else: 
      self.training_info = None
      """

    # assign a global ID
    global nextGlobalID
    self.globalID = nextGlobalID
    nextGlobalID += 1

  def getType(self):
    "Returns this term's output type"
    return self.functionTerm.value.outputType

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
    self.functionTerm.value.pythonEvaluate(self)

  def getIdentifier(self):
    return str(self.globalID)

  def printExtended(self, printer):
    printer.prints("%i: %s" % (self.globalID, self.functionTerm.value.name))

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



def findExisting(functionTerm, inputs=[]):
  """
  Assuming you want a term that has the given function and inputs, this function
  returns such a function if it exists and if it makes sense (according to the function's
  definition) to allow people to reuse a function.
  """
  if inputs is None:
    return None

  function = functionTerm.value

  pdb.set_trace()
  if function.shouldReuseExisting():
    # Try to find an existing term
    for input in inputs:
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
 
