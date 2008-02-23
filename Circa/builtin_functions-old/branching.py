import pdb

from Circa import (
  ca_function,
  builtins, 
  signature, 
  term_state
)

class CondBranch(ca_function.BaseFunction):
  name = "cond_branch"
  signature = signature.fixed(builtins.BOOL_TYPE)
  hasBranch = True

  def pythonEvaluate(self, term):
    if term.inputs[0].pythonValue:
      term.branch[0].pythonEvaluate()
    else:
      term.branch[1].pythonEvaluate()

class SimpleBranch(ca_function.BaseFunction):
  name = "simple_branch"
  signature = signature.empty()
  hasBranch = True

  def pythonEvaluate(self, term):
    for t in term.branch:
      t.pythonEvaluate()
