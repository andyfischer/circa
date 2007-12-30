import basefunction
import function_signature
import term_state

class CondBranch(basefunction.BaseFunction):
  def init(self):
    self.name = "cond_branch"
    self.signature = function_signature.specific(bool)

  def evaluate(self, term):
    if term.inputs[0].value:
      term.state.branches[0].evaluate()
    else:
      term.state.branches[1].evaluate()

  def makeState(self):
    return term_state.TermState(has_branch=True)


COND_BRANCH = CondBranch()

