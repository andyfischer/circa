

import term
import unittest
from term_state import TermState
from basefunction import BaseFunction

class SubroutineFunc(BaseFunction):
  def init(m):
    m.name = "subroutine"

  def evaluate(m, term):
    term.state.branches[0].evaluate()

  def makeState(m):
    return SubroutineState()


class SubroutineDefinition(object):
  def __init__(m):
    m.input_placeholders = []
    m.this_placeholder = term.create(functions.placeholder)
    m.state = SubroutineState()


  def addInput(m, name):
    term = term.create(functions.placeholder)
    m.state.putLocal(name, term)
    m.input_placeholders.append(term)
    return term


class SubroutineState(term.TermState):
  def __init__(m):
    m.locals = {}
    m.main_branch = m.addBranch()

  def putLocal(m, name, term):
    if name in m.locals:
      raise "Local with name " + str(name) + " already exists"

    m.locals[name] = term

  def getLocal(m, name):
    if not name in m.locals:
      return None
    else:
      return m.locals[name]


class Test(unittest.TestCase):
  def test1(m):
    f = SubroutineFunc()
    s = SubroutineState()
    d = SubroutineDefinition()

if __name__ == '__main__':
    unittest.main()
