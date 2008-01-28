

import ca_function, ca_types, signature

class Concatenate(ca_function.BaseFunction):
  name = "concat"
  signature = signature.fixed(ca_types.STRING, ca_types.STRING)

  def evaluate(self, term):
    term.value = str(term.inputs[0]) + str(term.inputs[1])
