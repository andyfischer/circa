

class BaseFunction(object):

  # Default options
  name = "undefined"
  pureFunction = False
  signature = None
  outputType = None
  hasBranch = False
  trainingType = None

  @classmethod
  def shouldReuseExisting(cls):
    return pureFunction

  def evaluate(self, term):
    pass

  def makeState(self):
    return None

