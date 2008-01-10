
ALL_TYPES = set((int, float, str, bool))

def assertSupported(type):
  if not type in ALL_TYPES:
    raise Exception("Unsupported type: " + str(type))

