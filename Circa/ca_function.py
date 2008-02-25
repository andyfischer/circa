
class Function(object):
  def __init__(self):
    self.inputTypes = []
    self.outputTypes = []
    self.hasBranch = False
    self.pureFunction = True

  def pythonEvaluate(self, term):
    pass


def createFunction(inputs, outputs):
  f = Function()
  f.inputTypes = inputs
  f.outputTypes = outputs
  return f

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

