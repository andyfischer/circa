import term
import builtin_functions
from term_state import TermState
import basefunction

class SubroutineFunc(basefunction.BaseFunction):
  def init(self):
    self.name = "subroutine"

  def makeState(self):
    return SubroutineState()


class SubroutineDefinition(object):
  def __init__(self):
    self.input_placeholders = []
    self.this_placeholder = term.Term(builtin_functions.PLACEHOLDER, branch=None)
    self.state = SubroutineState()

  def addInput(self, name):
    term = term.Term(builtin_functions.PLACEHOLDER)
    self.state.putLocal(name, term)
    self.input_placeholders.append(term)
    return term

class SubroutineState(TermState):
  def __init__(self):
    TermState.__init__(self, has_branch=True)
    self.locals = {}

  def putLocal(self, name, term):
    self.locals[name] = term

  def getLocal(self, name):
    if not name in self.locals:
      return None
    else:
      return self.locals[name]


SUBROUTINE = SubroutineFunc()
