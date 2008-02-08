
from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)

class Equal(ca_function.BaseFunction):
  name = "equals"
  pureFunction = True
  outputType = ca_types.BOOL


  
class NotEqual(ca_function.BaseFunction):
  name = "not_equals"
  pureFunction = True
  outputType = ca_types.BOOL
