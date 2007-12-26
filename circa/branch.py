

class Branch(list):
  def __new__(cls):
    self = list.__new__(cls, [])
    return self

  def evaluate(self):
    for term in self:
      if term.function:
        term.function.evaluate(term)
