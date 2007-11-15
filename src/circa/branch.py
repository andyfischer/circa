

class Branch(object):
  def __init__(m):
    m.terms = []

  def evaluate(m):
    for term in m.terms:
      if term.function:
        term.function.evaluate(term)

  def append(m, term):
    m.terms.append(term)
