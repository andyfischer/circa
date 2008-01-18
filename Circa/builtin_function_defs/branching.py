import pdb
import ca_function
import signature
import term_state

class CondBranch(ca_function.BaseFunction):
  def init(self):
    self.name = "cond_branch"
    self.signature = signature.fixed(bool)

  def evaluate(self, term):
    pdb.set_trace()
    if term.inputs[0].value:
      term.state.branch[0].evaluate()
    else:
      term.state.branch[1].evaluate()

  def makeState(self):
    return term_state.TermState(has_branch=True)

class SimpleBranch(ca_function.BaseFunction):
  def init(self):
    self.name = "simple_branch"
    self.signature = signature.empty()

  def evaluate(self, term):
    for t in term.state.branch:
      t.evaluate()

  def makeState(self):
    return term_state.TermState(has_branch=True)
