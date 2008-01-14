
import ca_function

class Print(ca_function.BaseFunction):
  def init(self):
    self.name = "print"
    
  def evaluate(self, term):
    print str(term.inputs[0])

