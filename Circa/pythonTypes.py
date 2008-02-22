


PYTHON_TO_CIRCA = {}

def pythonToCirca(type):
  if type in PYTHON_TO_CIRCA:
    return PYTHON_TO_CIRCA[type]
  else:
    return None

def typeOfPythonObj(obj):
  return pythonToCirca(type(obj))
