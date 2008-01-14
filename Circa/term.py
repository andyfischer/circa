import pdb

nextGlobalID = 1


class Term(object):
  def __init__(self, function, source_token=None):

    # initialize
    self.inputs = []
    self.function = function
    self.source_token = source_token
    self.users = set()
    self.value = None

    # make state
    state = self.function.makeState()

    # assign a global ID
    global nextGlobalID
    self.globalID = nextGlobalID
    nextGlobalID += 1

  @classmethod
  def createConstant(cls, value, **kwargs):
    import builtin_function_defs
    func = builtin_function_defs.Constant.fromType(type(value))
    term = Term(func, **kwargs)
    term.value = value
    return term

  @classmethod
  def createVariable(cls, value, **kwargs):
    import builtin_function_defs
    func = builtin_function_defs.Variable.fromType(type(value))
    term = Term(func, **kwargs)
    term.value = value
    return term

  def getType(self):
    "Returns this term's output type"
    return self.function.outputType


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

import builtin_function_defs

def placeholder():
  "Returns a new placeholder term"
  return Term(builtin_function_defs.PLACEHOLDER)

