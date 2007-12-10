

class Branch(object):
  def __init__(self):
    self.terms = []

  def evaluate(self):
    for term in self.terms:
      if term.function:
        term.function.evaluate(term)

  def append(self, term):
    self.terms.append(term)
