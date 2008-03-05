
import ca_function

def wrapPythonFunction(func, **options):
    circaFunc = ca_function.Function(**options)
    def funcForCirca(term):
        term.pythonValue = func(*map(lambda t:t.pythonValue, term.inputs))
    circaFunc.pythonEvaluate = funcForCirca
    return circaFunc


PYTHON_TYPE_TO_CIRCA = {}

def pythonTypeToCirca(type):
  if type in PYTHON_TYPE_TO_CIRCA:
    return PYTHON_TYPE_TO_CIRCA[type]
  else:
    return None

def typeOfPythonObj(obj):
  return pythonTypeToCirca(type(obj))

