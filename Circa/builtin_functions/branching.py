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

  def evaluate(self, term):
    if term.inputs[0].value:
      term.branch[0].evaluate()
    else:
      term.branch[1].evaluate()

class SimpleBranch(ca_function.BaseFunction):
  name = "simple_branch"
  signature = signature.empty()
  hasBranch = True

  def evaluate(self, term):
    for t in term.state.branch:
      t.evaluate()
