
import ca_function

class Print(ca_function.BaseFunction):
  name = "print"
    
  def evaluate(self, term):
    print str(term.inputs[0])

