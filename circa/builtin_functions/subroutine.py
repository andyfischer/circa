import term
import builtin_functions
from term_state import TermState
import basefunction

class SubroutineFunc(basefunction.BaseFunction):
  def init(m):
    m.name = "subroutine"

  def evaluate(m, term):
    term.state.branches[0].evaluate()

  def makeState(m):
    return SubroutineState()


class SubroutineDefinition(object):
  def __init__(m):
    m.input_placeholders = []
    m.this_placeholder = term.Term(builtin_functions.PLACEHOLDER, branch=None)
    m.state = SubroutineState()

  def addInput(m, name):
    term = term.Term(builtin_functions.PLACEHOLDER)
    m.state.putLocal(name, term)
    m.input_placeholders.append(term)
    return term

class SubroutineState(TermState):
  def __init__(m):
    m.locals = {}
    m.main_branch = m.addBranch()

  def putLocal(m, name, term):
    m.locals[name] = term

  def getLocal(m, name):
    if not name in m.locals:
      return None
    else:
      return m.locals[name]


SUBROUTINE = SubroutineFunc()
