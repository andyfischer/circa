

class CondBranch(TermFunction):

  def evaluate(m, term):
    if term.input[0].value:
      term.state.branches[0].evaluate()
    else:
      term.state.branches[1].evaluate()

  def makeState():
    return TermState(num_branches=2)

