import circa_function

class Constant(circa_function.BaseFunction):
  def init(self):
    self.name = "constant"

class Variable(circa_function.BaseFunction):
  def init(self):
    self.name = "variable"

CONSTANT = Constant()
VARIABLE = Variable()
