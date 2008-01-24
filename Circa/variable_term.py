import ca_function
import signature
import terms

type_to_function_instance = {}

class Variable(ca_function.BaseFunction):
  def init(self):
    self.name = "variable"
    self.signature = signature.empty()

def fromType(type):
  global type_to_function_instance

  "Returns a function instance that matches the given type"
  if type in type_to_function_instance:
    return type_to_function_instance[type]

  # make a new instance
  inst = Variable()
  inst.outputType = type
  inst.trainingType = type.trainingTypeAsSource
  type_to_function_instance[type] = inst
  return inst

def getValue(term):
  return term.state

def setValue(term, value):
  term.state = value
