
SUPPORTED_TYPES = set((int, float, str, bool))

def assertSupported(type):
  if not type in SUPPORTED_TYPES:
    raise Exception("Unsupported type: " + str(type))

