import pdb
import circa_types
import function_signature
import subroutine
from term_state import TermState
from basefunction import BaseFunction

from simple_math import *
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

class And(BaseFunction):
  def init(self):
    self.name = "and"
    self.inputType = bool
    self.outputType = bool

  def evaluate(self, term):
    for input in term.inputs:
      if not input.value:
        term.value = False
        return
    term.value = True

class Or(BaseFunction):
  def init(self):
    self.name = "or"
    self.inputType = bool
    self.outputType = bool

  def evaluate(self, term):
    for input in term.inputs:
      if input.value:
        term.value = True
        return
    term.value = False

class ConditionalExpression(BaseFunction):
  def init(self):
    self.name = "if"

  def evaluate(self, term):
    if term.inputs[0].value:
      term.value = term.inputs[1].value
    else:
      term.value = term.inputs[2].value


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
IF_EXPR = ConditionalExpression()
SUBROUTINE = subroutine.SubroutineFunc()
COND_BRANCH = CondBranch()

