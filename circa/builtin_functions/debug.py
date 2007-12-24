
from basefunction import BaseFunction

class Print(BaseFunction):
  def init(self):
    self.name = "print"
    
  def evaluate(self, term):
    print str(term.inputs[0])
