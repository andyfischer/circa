

class BaseFunction(object):
  def __init__(self):
    
    # default options
    self.name = None
    self.pureFunction = False
    self.signature = None
    self.outputType = None

    # init function, defined by implementors
    if hasattr(self, 'init'):
      self.init()

  def evaluate(self, term):
    pass

  def makeState(self):
    return None

class Unspecialized(object):
  def __init__(self):
    self.name = None
    self.signature_function_pairs = []

  def addEntry(self, sig, func):
    assert isinstance(sig, signature.Signature)
    assert isinstance(func, circa_function.BaseFunction)

    self.signature_function_pairs.append((sig,func))

  def specialize(self, term):
    for (sig,func) in self.signature_function_pairs:
      if sig.satisfiedBy(term):
        return func

    return None

