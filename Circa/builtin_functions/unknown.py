
from Circa import (
  ca_function,
  signature
)

# Causes
(NAME_NOT_FOUND, NAME_NOT_A_FUNCTION) = range(2)

def nameNotAFunction(name):
  return UnknownFunction(name, NAME_NOT_A_FUNCTION)

def nameNotFound(name):
  return UnknownFunction(name, NAME_NOT_FOUND)

class UnknownFunction(ca_function.BaseFunction):
  def __init__(self, given_name, cause):
    ca_function.BaseFunction.__init__(self)
    self.name = given_name
    self.cause = cause

    self.signature = signature.anything()
    self.outputType = None

  def pythonEvaluate(self, term):
    print "Warning: tried to evaluate unknown function: " + str(self.name)
