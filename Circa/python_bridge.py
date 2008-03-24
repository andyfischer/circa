

PYTHON_TYPE_TO_CIRCA = {}

def pythonTypeToCirca(type):
   if type in PYTHON_TYPE_TO_CIRCA:
      return PYTHON_TYPE_TO_CIRCA[type]
   else:
      return None

def typeOfPythonObj(obj):
   return pythonTypeToCirca(type(obj))
