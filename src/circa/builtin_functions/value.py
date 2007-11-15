
import functions

class Constant(functions.BaseFunction):
  def init(m):
    m.name = "constant"

class Variable(functions.BaseFunction):
  def init(m):
    m.name = "variable"


