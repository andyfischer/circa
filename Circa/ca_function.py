
class Function(object):
  def __init__(self, pureFunction=True):
    self.inputTypes = []
    self.outputTypes = []
    self.hasBranch = False
    self.pureFunction = pureFunction
    self.forceUnique = False

  def pythonEvaluate(self, term):
    pass


def createFunction(inputs, outputs):
  f = Function()
  f.inputTypes = inputs
  f.outputTypes = outputs
  return f

def createUnknownFunction(name):
  f = Function()
  f.name = name
  return f

