import circa.functions
import unittest

class Term(object):
  def __init__(m, initial_inputs=[]):

    m.function = None
    m.inputs = []
    m.users = set()
    m.value = None

    m.setInputs(initial_inputs)

  def setInputs(m, list):
    old_inputs = m.inputs

    m.inputs = list[:]

    # add to user lists
    for input in m.inputs:
      input.users.add(m)

    # prune old list
    for input in old_inputs:
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



def create(func, branch, inputs=[], initial_state=None):
  # todo: specialize function

  term = Term()

  term.func = func

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

def createConstant(value, branch):
  term = createTerm(functions.constant, branch)
  term.value = value
  return term

def createVariable(value, branch):
  term = createTerm(functions.variable, branch)
  term.value = value
  return term


class TestTermCase(unittest.TestCase):
  def testSimple(m):
    br = branch.branch()


    one = term.createConstant(1, br)
    two = term.createConstant(2, br)
    three = term.create(functions.builtin.add, br, [one,two])

    br.evaluate()

    assert three.value == 3
