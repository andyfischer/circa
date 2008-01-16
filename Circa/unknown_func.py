
import ca_function
import signature

# Causes
(NAME_NOT_FOUND, NAME_NOT_A_FUNCTION) = range(2)

class UnknownFunction(ca_function.BaseFunction):
  def __init__(self, given_name, cause):
    ca_function.BaseFunction.__init__(self)
    self.name = given_name
    self.cause = cause

    self.signature = signature.anything()
    self.outputType = None
