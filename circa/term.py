import unittest
from term_state import TermState

nextStaticID = 1


class Term(object):
  def __init__(self, initial_inputs=[]):

    self.function = None
    self.inputs = []
    self.users = set()
    self.value = None
    self.state = None

    global nextStaticID
    self.staticID = nextStaticID
    nextStaticID += 1

    self.setInputs(initial_inputs)

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
    for input in self.inputs:
      if input == term: return True
    return false

  # pruneUsers: Removes any terms from our user list that are not really using us
  def pruneUsers(self):
    for user in self.users:
      if not user.inputsContain(self):
        self.users.remove(user)

  def changeFunction(self, new_func, initial_state=None):
    self.function = new_func

    if not initial_state:
      self.state = self.function.makeState()
    else:
      self.state = initial_state

  def evaluate(self):
    self.function.evaluate(self)

  def printExtended(self, printer):
    printer.prints("%i: %s" % (self.staticID, self.function.name))

    if self.value:
      printer.prints(" " + str(self.value))

    printer.prints(" [")

    first_item = True
    for input in self.inputs:
      if not first_item: printer.prints(",")
      printer.prints(input.staticID)
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

  # value accessors
  def __int__(self): return int(self.value)
  def __float__(self): return float(self.value)
  def __str__(self): return str(self.value)

  # member accessor
  def __getitem__(self, name):
    return self.state.getLocal(name)

from branch import Branch
from basefunction import BaseFunction

def create(func, branch=None, inputs=[], initial_state=None):
  assert isinstance(func, BaseFunction)

  # todo: specialize function

  term = Term()

  term.function = func

  # set state
  if initial_state:
    term.state = initial_state
  else:
    term.state = func.makeState()

  # set inputs
  if inputs: term.setInputs(inputs)

  # add to branch
  if branch:
    branch.append(term)

  # evaluate
  term.evaluate()

  return term

import builtin_functions

def createConstant(value, branch):
  term = create(builtin_functions.CONSTANT, branch)
  term.value = value
  return term

def createVariable(value, branch):
  term = create(builtin_functions.VARIABLE, branch)
  term.value = value
  return term

