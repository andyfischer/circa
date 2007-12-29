

class Branch(list):
  def __new__(cls):
    self = list.__new__(cls, [])
    return self

