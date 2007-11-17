
from base_function import BaseFunction
import circa.data_types as data_types

class And(BaseFunction):
  def init(m):
    m.name = "and"
    m.inputType = types.BOOL
    m.outputType = types.BOOL

  def evaluate(m, term):
    for input in term.inputs:
      if not input.value:
        term.value = False
        return
    term.value = True
    return

class Or(BaseFunction):
  def init(m):
    m.name = "or"
    m.inputType = data_types.BOOL
    m.outputType = data_types.BOOL

  def evaluate(m, term):
    for input in term.inputs:
      if input.value:
        term.value = True
        return
    term.value = False
    return

class ConditionalExpression(BaseFunction):
  def init(m):
    m.name = "if"

  def evaluate(m, term):
    if term.input[0].value:
      term.value = term.input[1]
    else:
      term.value = term.input[2]
