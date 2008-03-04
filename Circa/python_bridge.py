
import ca_function

def wrapPythonFunction(func):
    circaFunc = ca_function.Function()
    def funcForCirca(term):
        try:
            term.pythonValue = func(*map(lambda t:t.pythonValue, term.inputs))
        except:
            pass
    circaFunc.pythonEvaluate(funcForCirca)
    return circaFunc


PYTHON_TYPE_TO_CIRCA = {}

def pythonTypeToCirca(type):
  if type in PYTHON_TYPE_TO_CIRCA:
    return PYTHON_TYPE_TO_CIRCA[type]
  else:
    return None

def typeOfPythonObj(obj):
  return pythonTypeToCirca(type(obj))

