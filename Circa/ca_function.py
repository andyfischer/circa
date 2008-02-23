
class Function(object):
  def __init__(self):
    self.inputTypes = []
    self.outputTypes = []


def createFunction(inputs, outputs):
  f = Function()
  f.inputTypes = inputTypes
  f.outputTypes = outputTypes

# Deprecated:
class BaseFunction(object):

  # Default options
  name = "undefined"
  pureFunction = False
  signature = None
  outputType = None
  hasBranch = False
  trainingType = None
  onCreate = None
  onDestroy = None

  @classmethod
  def shouldReuseExisting(cls):
    return cls.pureFunction

  def pythonEvaluate(self, term):
    pass

  def makeState(self):
    return None

