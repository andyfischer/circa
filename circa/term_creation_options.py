

class TermCreationOptions(object):
  def __init__(self, function, inputs=[], state=None, source_token=None):
    self.inputs = inputs
    self.state = state
    self.source_token = source_token

    if state and function.pureFunction:
      raise UsageError("Found state object when using a pure function")
