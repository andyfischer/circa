
from base_function import BaseFunction


class Add(BaseFunction):
  def evaluate(m, term):
    term.value = term.inputs[0].value + term.inputs[1].value

class Sub(BaseFunction):
  def evaluate(m, term):
    term.value = term.inputs[0].value - term.inputs[1].value

class Mult(BaseFunction):
  def evaluate(m, term):
    term.value = term.inputs[0].value * term.inputs[1].value

class Div(BaseFunction):
  def evaluate(m, term):
    term.value = term.inputs[0].value / term.inputs[1].value
    
