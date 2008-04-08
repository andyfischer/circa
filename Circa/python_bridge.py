

_PYTHON_TYPE_TO_CIRCA = {}

def registerType(pythonType, circaType):
   _PYTHON_TYPE_TO_CIRCA[pythonType] = circaType

def pythonTypeToCirca(type):
   if type in _PYTHON_TYPE_TO_CIRCA:
      return _PYTHON_TYPE_TO_CIRCA[type]
   else:
      return None

def typeOfPythonObj(obj):
   return pythonTypeToCirca(type(obj))

def wrapPythonFunction(pythonFunc):
   """
   This function wraps a Python function so that it is suitable to be used
   as the 'pythonEvaluate' portion of an existing Function. The values of
   all the arguments of the Circa term are copied as arguments to the Python
   function, and the returned result of the Python function is used as the
   'pythonValue' of the Circa term.
   """

   def funcForCirca(term):
       term.pythonValue = pythonFunc(*map(lambda t:t.pythonValue, term.inputs))
   return funcForCirca

