

class BaseFunction(object):
  def __init__(self):
    
    # default options
    self.name = None
    self.pureFunction = False
    self.signature = None
    self.outputType = None
    self.hasBranch = False

    # init function, defined by implementors
    if hasattr(self, 'init'):
      self.init()

  def evaluate(self, term):
    pass

  def makeState(self):
    return None

