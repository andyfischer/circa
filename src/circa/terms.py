import unittest

class Term(object):
  def __init__(m, initial_inputs=[]):

    m.function = None
    m.inputs = []
    m.users = set()
    m.value = None

    m.setInputs(initial_inputs)

  def input(m, index):
    return m.inputs[index].value

  def setInputs(m, list):
    old_inputs = m.inputs

    m.inputs = list[:]

    # add to user lists
    for input in m.inputs:
      if not input: continue
      input.users.add(m)

    # prune old list
    for input in old_inputs:
      if not input: continue
      input.pruneUsers()

  def setInput(m, index, term):
    old_input = m.inputs[input]
    m.inputs[input] = term

    if term != None: term.users.add(m)

    old_input.pruneUsers()

  def inputsContain(m, term):
    for input in m.inputs:
      if input == term: return True
    return false

  # pruneUsers: Removes any terms from our user list that are not really using us
  def pruneUsers(m):
    for user in m.users:
      if not user.inputsContain(m):
        m.users.remove(user)

  def changeFunction(m, new_func, initial_state=None):
    m.function = new_func

    if not initial_state:
      m.state = m.function.makeState()
    else:
      m.state = initial_state

  def evaluate(m):
    m.function.evaluate(m)

from branch import Branch

class TermState(object):
  def __init__(m, num_branches=0):

    if num_branches > 0:
      for i in range(num_branches): m.addBranch()


  def addBranch(m):
    if not hasattr(m, 'branches'):
      m.branches = []

    branch = Branch()
    m.branches.append(branch)
    return branch


def create(func, branch=None, inputs=[], initial_state=None):
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

  return term

import functions

def createConstant(value, branch):
  term = create(functions.constant, branch)
  term.value = value
  return term

def createVariable(value, branch):
  term = create(functions.variable, branch)
  term.value = value
  return term



