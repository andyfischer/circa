import pdb
import circa_function

nextGlobalID = 1


def placeholder():
  "Returns a new placeholder term"
  return Term(builtin_functions.PLACEHOLDER)

class Term(object):
  __slots__ = ['function', 'inputs', 'users', 'state', 'source_token', 'outputType']

  def __init__(self, function, inputs=[], state=None, source_token=None):

    # initialize
    self.function = function
    self.inputs = []
    self.source_token = source_token

    # make state, or use one that was passed
    if not state:
      state = self.function.makeState()
    self.state = state

    self.setInputs(inputs)

    self.outputType = None
    self.outputTypeOutOfDate = True
    self.users = set()
    self.value = None

    # assign a global ID
    global nextGlobalID
    self.globalID = nextGlobalID
    nextGlobalID += 1

    # evaluate immediately, if this is a pure function
    if function.pureFunction:
      self.evaluate()
      
      """
  @classmethod
  def createConstant(cls, value, **kwargs):
    import builtin_functions
    term = Term(builtin_functions.CONSTANT, **kwargs)
    term.value = value
    return term

  @classmethod
  def createVariable(cls, value, **kwargs):
    import builtin_functions
    term = Term(builtin_functions.VARIABLE, **kwargs)
    term.value = value
    return term
    """

  def input(self, index):
    return self.inputs[index].value

  def getType(self):
    "Returns this term's output type"
    
    if self.outputTypeOutOfDate:
      self.outputType = self.function.getOutputType(self)

    return self.outputType


  def inputsContain(self, term):
    "Returns True if our inputs contain the term"
    for input in self.inputs:
      if input == term: return True
    return false

  # pruneUsers: Removes any terms from our user list that are not really using us
  def pruneUsers(self):
    """
    Go through this term's user list, remove anything there that is no longer
    using us.
    """
    for user in self.users:
      if not user.inputsContain(self):
        self.users.remove(user)

  def changeFunction(self, new_func, initial_state=None):
    "Change this term's function"
    self.function = new_func

    if not initial_state:
      self.state = self.function.makeState()
    else:
      self.state = initial_state

  def evaluate(self):
    self.function.evaluate(self)

  def printExtended(self, printer):
    printer.prints("%i: %s" % (self.globalID, self.function.name))

    if self.value:
      printer.prints(" " + str(self.value))

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

  def iterator(self, forwards=True):
    yield self
    if self.state:
      for branch in self.branches:
        for term in branch:
          yield term

  # value accessors
  def __int__(self): return int(self.value)
  def __float__(self): return float(self.value)
  def __str__(self): return str(self.value)

  # member accessor
  def __getitem__(self, name):
    return self.state.getLocal(name)


