
from base_function import BaseFunction
import circa.types as types

class And(BaseFunction):
  def init(m):
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
    m.inputType = types.BOOL
    m.outputType = types.BOOL

  def evaluate(m, term):
    for input in term.inputs:
      if input.value:
        term.value = True
        return
    term.value = False
    return

class ConditionalExpression(BaseFunction):

  def evaluate(m, term):
    if term.input[0].value:
      term.value = term.input[1]
    else:
      term.value = term.input[2]
