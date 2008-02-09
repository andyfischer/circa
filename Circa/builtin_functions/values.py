
from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)

class Constant(ca_function.BaseFunction):
  type_to_function_instance = {}

  name = "constant"
  input = signature.empty()

  @classmethod
  def fromType(cls, type):
    "Returns a function instance that matches the given type"
    if type in cls.type_to_function_instance:
      return cls.type_to_function_instance[type]

    # make a new instance
    inst = Constant()
    inst.outputType = type
    Constant.type_to_function_instance[type] = inst
    return inst

  @classmethod
  def isConstant(cls, term):
    "Returns True if the term is a constant term"
    return isinstance(term.function, Constant)
  
class Variable(ca_function.BaseFunction):
  type_to_function_instance = {}

  name = "variable"
  input = signature.empty()

  @classmethod
  def fromType(cls, type):
    "Returns a function instance that matches the given type"
    if type in cls.type_to_function_instance:
      return cls.type_to_function_instance[type]

    # make a new instance
    inst = Variable()
    inst.outputType = type
    inst.trainingType = type.trainingTypeAsSource
    Variable.type_to_function_instance[type] = inst
    return inst

  @classmethod
  def getValue(cls, term):
    return term.state

  @classmethod
  def setValue(cls, term, value):
    term.state = value


