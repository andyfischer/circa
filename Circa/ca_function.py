

class BaseFunction(object):
  # Default options
  name = None
  pureFunction = False
  signature = None
  outputType = None
  hasBranch = False
  trainingType = None

  def evaluate(self, term):
    pass

  def makeState(self):
    return None

