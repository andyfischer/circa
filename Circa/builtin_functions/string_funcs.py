
from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)


class Concatenate(ca_function.BaseFunction):
  name = "concat"
  signature = signature.fixed(ca_types.STRING, ca_types.STRING)

  def evaluateEmulated(self, term):
    term.pythonValue = str(term.inputs[0]) + str(term.inputs[1])
