import ca_function
import training

class BaseType(object):
  trainingTypeAsSource = None

class CircaInt(BaseType):
  pythonType = int

class CircaFloat(BaseType):
  pythonType = float
  trainingTypeAsSource = training.NumericalSource

class CircaString(BaseType):
  pythonType = str

class CircaBool(BaseType):
  pythonType = bool

class CircaFunction(BaseType):
  pass

INT = CircaInt
FLOAT = CircaFloat
STRING = CircaString
BOOL = CircaBool
FUNC = CircaFunction

ALL_TYPES = set((CircaInt, CircaFloat, CircaString, CircaBool, CircaFunction))

def assertSupported(type):
  if not type in ALL_TYPES:
    raise Exception("Unsupported type: " + str(type))


def getTypeOfPythonObj(obj):
  if isinstance(obj, ca_function.BaseFunction):
    return FUNC
  elif isinstance(obj, int):
    return CircaInt
  elif isinstance(obj, float):
    return CircaFloat
  elif isinstance(obj, str):
    return CircaString
  elif isinstance(obj, bool):
    return CircaBool

  raise Exception("Not recognized")
