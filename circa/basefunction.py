


class BaseFunction(object):
  def __init__(self):
    
    # default options
    self.name = None
    self.pureFunction = False
    self.numBranches = 0
    self.numTermPointers = 0
    self.inputType = None
    self.outputType = None

    self.init()

    # make sure they specified a name
    assert self.name != None

  def init(self):
    pass

  def evaluate(self, term):
    pass

  def makeState(self):
    return None


