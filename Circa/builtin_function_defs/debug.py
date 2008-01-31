
from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)


class Print(ca_function.BaseFunction):
  name = "print"
    
  def evaluate(self, term):
    print str(term.inputs[0])

class GetInput(ca_function.BaseFunction):
  name = "input"
  signature = signature.empty()
  outputType = ca_types.STRING

  def evaluate(self, term):
    term.value = raw_input("> ")
