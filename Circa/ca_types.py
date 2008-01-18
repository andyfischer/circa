import ca_function

FUNC = "function"

ALL_TYPES = set((int, float, str, bool, FUNC))

def assertSupported(type):
  if not type in ALL_TYPES:
    raise Exception("Unsupported type: " + str(type))


def getType(obj):
  # Treat all functions as a single type
  if isinstance(obj, ca_function.BaseFunction):
    return FUNC

  return type(obj)
