

class BaseFunction(object):
  __slots__ = ['name', 'pureFunction', 'signature', 'outputType']

  def __init__(self):
    
    # default options
    self.name = None
    self.pureFunction = False
    self.signature = None
    self.outputType = None

    # init function, defined by implementors
    self.init()

    # make sure they specified a name
    assert self.name != None

  def evaluate(self, term):
    pass

  def getOutputType(self, term):

  def makeState(self):
    return None


