
import ca_function
import signature
import terms

type_to_function_instance = {}

def isConstant(term):
  "Returns True if the term is a constant term"
  return isinstance(term.function, Constant)

def fromType(type):
  global type_to_function_instance

  "Returns a function instance that matches the given type"
  if type in type_to_function_instance:
    return type_to_function_instance[type]

  # make a new instance
  inst = Constant()
  inst.outputType = type
  type_to_function_instance[type] = inst
  return inst


class Constant(ca_function.BaseFunction):
  def init(self):
    self.name = "constant"
    self.signature = signature.empty()

