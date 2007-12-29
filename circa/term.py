import pdb

import basefunction
import builtin_functions
from term_state import TermState

nextGlobalID = 1


class Term(object):
  def __init__(self, function, inputs=[], branch="unspecified", initial_state=None, source_token=None):
    assert isinstance(function, basefunction.BaseFunction)
    assert branch != "unspecified"

    # initialize
    self.function = function
    self.inputs = []
    self.users = set()
    self.value = None
    self.state = None
    self.source_token = source_token

    # set inputs
    if inputs:
      self.setInputs(inputs)

    # set state
    if initial_state:
      self.state = initial_state
    else:
      self.state = function.makeState()

    if self.state and function.pureFunction:
      raise UsageError("Found state object when using a pure function")

    # assign a global ID
    global nextGlobalID
    self.globalID = nextGlobalID
    nextGlobalID += 1

    # add to a branch
    if branch != None:
      branch.append(self)

    # evaluate immediately
    if function.pureFunction:
      self.evaluate()
      
  @classmethod
  def createConstant(cls, value, **kwargs):
    term = Term(builtin_functions.CONSTANT, **kwargs)
    term.value = value
    return term

  def createVariable(cls, value, **kwargs):
    term = Term(builtin_functions.VARIABLE, **kwargs)
    term.value = value
    return term

  def input(self, index):
    return self.inputs[index].value

  def setInputs(self, list):
    old_inputs = self.inputs

    self.inputs = list[:]

    # add to user lists
    for input in self.inputs:
      if not input: continue
      input.users.add(self)

    # prune old list
    for input in old_inputs:
      if not input: continue
      input.pruneUsers()

  def setInput(self, index, term):
    old_input = self.inputs[input]
    self.inputs[input] = term

    if term != None: term.users.add(self)

    old_input.pruneUsers()

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


