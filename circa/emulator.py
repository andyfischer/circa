


def evaluateBranch(term):
  for inner_term in term.state.branch:
    inner_term.evaluate()

    if inner_term.state and inner_term.state.branch:
      evaluateBranch(inner_term)
