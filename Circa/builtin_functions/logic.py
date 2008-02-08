
from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)

class And(ca_function.BaseFunction):
  name = "and"
  pureFunction = True
  inputType = ca_types.BOOL
  outputType = ca_types.BOOL

  def pythonEvaluate(self, term):
    for input in term.inputs:
      if not input.pythonValue:
        term.pythonValue = False
        return
    term.pythonValue = True

class Or(ca_function.BaseFunction):
  name = "or"
  pureFunction = True
  inputType = ca_types.BOOL
  outputType = ca_types.BOOL

  def pythonEvaluate(self, term):
    for input in term.inputs:
      if input.pythonValue:
        term.pythonValue = True
        return
    term.pythonValue = False

class ConditionalExpression(ca_function.BaseFunction):
  name = "if"
  pureFunction = True

  def pythonEvaluate(self, term):
    if term.inputs[0].pythonValue:
      term.pythonValue = term.inputs[1].pythonValue
    else:
      term.pythonValue = term.inputs[2].pythonValue

