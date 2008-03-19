
import ca_function

def wrapPythonFunction(func, **options):
   """
   This function returns an instance of Function (using named arguments in the
   constructor, with 'func' implanted as its evaluation function.
   """
   circaFunc = ca_function.Function(**options)
   circaFunc.pythonEvaluate = wrapPythonFuncToEvaluate(funcForCirca)
   return circaFunc

def wrapPythonFuncToEvaluate(func):
   """
   This function wraps a Python function so that it is suitable to be used
   as the 'pythonEvaluate' portion of an existing Function.
   """

   def funcForCirca(term):
       term.pythonValue = func(*map(lambda t:t.pythonValue, term.inputs))
   return funcForCirca


PYTHON_TYPE_TO_CIRCA = {}

def pythonTypeToCirca(type):
   if type in PYTHON_TYPE_TO_CIRCA:
      return PYTHON_TYPE_TO_CIRCA[type]
   else:
      return None

def typeOfPythonObj(obj):
   return pythonTypeToCirca(type(obj))
