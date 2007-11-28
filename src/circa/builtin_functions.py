import terms
from basefunction import BaseFunction



# class definitions
class CondBranch(BaseFunction):

  def evaluate(m, term):
    if term.input[0].value:
      term.state.branches[0].evaluate()
    else:
      term.state.branches[1].evaluate()

  def makeState():
    return terms.TermState(num_branches=2)

class And(BaseFunction):
  def init(m):
    m.name = "and"
    m.inputType = bool
    m.outputType = bool

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
    m.inputType = bool
    m.outputType = bool

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

class Add(BaseFunction):
  def init(m):
    m.name = "add"
    m.inputType = float
    m.outputType = float

  def evaluate(m, term):
    term.value = float(term.inputs[0]) + float(term.inputs[1])

class Sub(BaseFunction):
  def init(m):
    m.name = "sub"
    m.inputType = float
    m.outputType = float

  def evaluate(m, term):
    term.value = float(term.inputs[0]) - float(term.inputs[1])

class Mult(BaseFunction):
  def init(m):
    m.name = "mult"
    m.inputType = float
    m.outputType = float

  def evaluate(m, term):
    term.value = float(term.inputs[0]) * float(term.inputs[1])

class Div(BaseFunction):
  def init(m):
    m.name = "div"
    m.inputType = float
    m.outputType = float

  def evaluate(m, term):
    term.value = float(term.inputs[0]) / float(term.inputs[1])
    
class Blend(BaseFunction):
  def init(m):
    m.name = "blend"
    m.inputType = float
    m.outputType = float

  def evaluate(m, term):
    blend_value = float(term.inputs[2])
    term.value = float(term.inputs[0]) * (1 - blend_value) + float(term.inputs[1]) * blend_value

class Placeholder(BaseFunction):
  def init(m):
    m.name = "placeholder"

class Constant(BaseFunction):
  def init(m):
    m.name = "constant"

class Variable(BaseFunction):
  def init(m):
    m.name = "variable"

class Subroutine(BaseFunction):
  def init(m):
    m.name = "subroutine"

  def makeState(m):
    return terms.TermState(num_branches=1)

  def evaluate(m, term):
    term.state.branches[0].evaluate()

# global function instances

add = Add()
sub = Sub()
mult = Mult()
div = Div()
blend = Blend()
constant = Constant()
variable = Variable()
subroutine = Subroutine()

