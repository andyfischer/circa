
import basefunction

class Constant(basefunction.BaseFunction):
  def init(self):
    self.name = "constant"

class Variable(basefunction.BaseFunction):
  def init(self):
    self.name = "variable"

CONSTANT = Constant()
VARIABLE = Variable()
