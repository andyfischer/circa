import ca_function
import signature
import term_state

class CondBranch(ca_function.BaseFunction):
  def init(self):
    self.name = "cond_branch"
    self.signature = signature.fixed(bool)

  def evaluate(self, term):
    if term.inputs[0].value:
      term.state.branches[0].evaluate()
    else:
      term.state.branches[1].evaluate()

  def makeState(self):
    return term_state.TermState(has_branch=True)


COND_BRANCH = CondBranch()

