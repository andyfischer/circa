import circa_function
import signature

class Constant(circa_function.BaseFunction):
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

class Variable(circa_function.BaseFunction):
  def init(self):
    self.name = "variable"
    self.signature = signature.empty()


CONSTANT = Constant()
VARIABLE = Variable()
