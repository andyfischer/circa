

from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)

class Assign(ca_function.BaseFunction):
  pureFunction = False

  def pythonEvaluate(self, term):
    term.inputs[0].pythonValue = term.inputs[1].pythonValue
