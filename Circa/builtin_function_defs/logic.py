
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

  def evaluate(self, term):
    for input in term.inputs:
      if not input.value:
        term.value = False
        return
    term.value = True

class Or(ca_function.BaseFunction):
  name = "or"
  pureFunction = True
  inputType = ca_types.BOOL
  outputType = ca_types.BOOL

  def evaluate(self, term):
    for input in term.inputs:
      if input.value:
        term.value = True
        return
    term.value = False

class ConditionalExpression(ca_function.BaseFunction):
  name = "if"
  pureFunction = True

  def evaluate(self, term):
    if term.inputs[0].value:
      term.value = term.inputs[1].value
    else:
      term.value = term.inputs[2].value

