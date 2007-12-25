import pdb
import circa_types
import function_signature
import subroutine
from term_state import TermState
from basefunction import BaseFunction

from simple_math import *
from logic import *
from debug import *


# class definitions
class CondBranch(BaseFunction):
  def init(self):
    self.name = "cond_branch"
    self.signature = function_signature.specific(bool)

  def evaluate(self, term):
    if term.inputs[0].value:
      term.state.branches[0].evaluate()
    else:
      term.state.branches[1].evaluate()

  def makeState(self):
    return TermState(num_branches=2)


class Placeholder(BaseFunction):
  def init(self):
    self.name = "placeholder"

class Constant(BaseFunction):
  def init(self):
    self.name = "constant"

class Variable(BaseFunction):
  def init(self):
    self.name = "variable"

# global function instances

CONSTANT = Constant()
VARIABLE = Variable()
SUBROUTINE = subroutine.SubroutineFunc()
COND_BRANCH = CondBranch()
PLACEHOLDER = Placeholder()

