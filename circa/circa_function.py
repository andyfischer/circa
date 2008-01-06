

class BaseFunction(object):
  def __init__(self):
    
    # default options
    self.name = None
    self.pureFunction = False
    self.signature = None
    self.fixedOutputType = None
    self.specializeTypes = None

    # init function, defined by implementors
    self.init()

    # make sure they specified a name
    assert self.name != None

  def evaluate(self, term):
    pass

  def makeState(self):
    return None


