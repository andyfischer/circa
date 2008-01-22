import ca_function

class CircaType(object):
  pass

class CircaInt(CircaType):
  def makePythonObject(self):
    return int()

class CircaFloat(CircaType):
  def makePythonObject(self):
    return float()

class CircaString(CircaType):
  def makePythonObject(self):
    return str()

class CircaBool(CircaType):
  def makePythonObject(self):
    return bool()

class CircaFunction(CircaType):
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
