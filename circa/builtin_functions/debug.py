
import circa_function

class Print(circa_function.BaseFunction):
  def init(self):
    self.name = "print"
    
  def evaluate(self, term):
    print str(term.inputs[0])
