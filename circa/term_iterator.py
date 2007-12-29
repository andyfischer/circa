

def iterate(term):
  yield term

  for branch in term.state.branches:
    for term in branch:
      yield term

