
from base_function import BaseFunction


class Add(BaseFunction):
  def init(m):
    m.name = "add"

  def evaluate(m, term):
    term.value = term.inputs(0) + term.inputs(1)

class Sub(BaseFunction):
  def init(m):
    m.name = "sub"

  def evaluate(m, term):
    term.value = term.input(0) - term.inputs(1)

class Mult(BaseFunction):
  def init(m):
    m.name = "mult"

  def evaluate(m, term):
    term.value = term.input(0) * term.input(1)

class Div(BaseFunction):
  def init(m):
    m.name = "div"

  def evaluate(m, term):
    term.value = term.input(0) / term.input(1)
    
class Blend(BaseFunction):
  def init(m):
    m.name = "blend"

  def evaluate(m, term):
    blend_value = term.input(2)
    term.value = term.input(0) * (1 - blend_value) + term.input(1)
