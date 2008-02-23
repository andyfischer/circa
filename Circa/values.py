
def isConstant(term):
  return isinstance(term.function, value.Constant)

typeToConstantFunction = {}
typeToVariableFunction = {}

def constFunctionFromType(type):
  if type in typeToConstantFunction:
    return typeToConstantFunction[type]

  # make a new instance
  inst = value.Constant()
  inst.outputType = type
  typeToConstantFunction[type] = inst
  return inst
 
def variableFunctionFromType(type):
  if type in typeToVariableFunction:
    return typeToVariableFunction[type]

  # make a new instance
  inst = value.Variable()
  inst.outputType = type
  typeToVariableFunction[type] = inst
  return inst
