import pdb

import ca_function
import signature

def isConstant(term):
  return isinstance(term.function, Constant)

class Constant(ca_function.BaseFunction):
  def init(self):
    self.name = "constant"
    self.signature = signature.empty()

  type_to_instance = {}

  @classmethod
  def fromType(cls, type):
    if type in Constant.type_to_instance:
      return Constant.type_to_instance[type]

    # make a new instance
    obj = cls()
    obj.outputType = type
    Constant.type_to_instance[type] = obj
    return obj

class Variable(ca_function.BaseFunction):
  def init(self):
    self.name = "variable"
    self.signature = signature.empty()


