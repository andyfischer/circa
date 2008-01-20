import ca_function
import signature
import terms

class Variable(ca_function.BaseFunction):
  def init(self):
    self.name = "variable"
    self.signature = signature.empty()


