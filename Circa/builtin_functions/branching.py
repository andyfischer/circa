import pdb

from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)

class CondBranch(ca_function.BaseFunction):
  name = "cond_branch"
  signature = signature.fixed(ca_types.BOOL)
  hasBranch = True

  def evaluateEmulated(self, term):
    if term.inputs[0].pythonValue:
      term.branch[0].evaluateEmulated()
    else:
      term.branch[1].evaluateEmulated()

class SimpleBranch(ca_function.BaseFunction):
  name = "simple_branch"
  signature = signature.empty()
  hasBranch = True

  def evaluateEmulated(self, term):
    for t in term.state.branch:
      t.evaluateEmulated()
