
from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)

class Constant(ca_function.BaseFunction):
  type_to_function_instance = {}

  name = "constant"
  input = signature.empty()
  
class Variable(ca_function.BaseFunction):
  type_to_function_instance = {}

  name = "variable"
  input = signature.empty()

