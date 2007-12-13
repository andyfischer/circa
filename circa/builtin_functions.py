import subroutine
from term_state import TermState
from basefunction import BaseFunction



# class definitions
class CondBranch(BaseFunction):
  def init(self):
    self.name = "cond_branch"

  def evaluate(self, term):
    if term.inputs[0].value:
      term.state.branches[0].evaluate()
    else:
      term.state.branches[1].evaluate()

  def makeState(self):
    return TermState(num_branches=2)

class And(BaseFunction):
  def init(self):
    self.name = "and"
    self.inputType = bool
    self.outputType = bool

  def evaluate(self, term):
    for input in term.inputs:
      if not input.value:
        term.value = False
        return
    term.value = True
    return

class Or(BaseFunction):
  def init(self):
    self.name = "or"
    self.inputType = bool
    self.outputType = bool

  def evaluate(self, term):
    for input in term.inputs:
      if input.value:
        term.value = True
        return
    term.value = False
    return

class ConditionalExpression(BaseFunction):
  def init(self):
    self.name = "if"

  def evaluate(self, term):
    if term.inputs[0].value:
      term.value = term.inputs[1].value
    else:
      term.value = term.inputs[2].value

class Add(BaseFunction):
  def init(self):
    self.name = "add"
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    term.value = float(term.inputs[0]) + float(term.inputs[1])

class Sub(BaseFunction):
  def init(self):
    self.name = "sub"
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    term.value = float(term.inputs[0]) - float(term.inputs[1])

class Mult(BaseFunction):
  def init(self):
    self.name = "mult"
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    term.value = float(term.inputs[0]) * float(term.inputs[1])

class Div(BaseFunction):
  def init(self):
    self.name = "div"
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    term.value = float(term.inputs[0]) / float(term.inputs[1])
    
class Blend(BaseFunction):
  def init(self):
    self.name = "blend"
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    blend_value = float(term.inputs[2])
    term.value = float(term.inputs[0]) * (1 - blend_value) + float(term.inputs[1]) * blend_value

class Placeholder(BaseFunction):
  def init(self):
    self.name = "placeholder"

class Constant(BaseFunction):
  def init(self):
    self.name = "constant"

class Variable(BaseFunction):
  def init(self):
    self.name = "variable"

# global function instances

ADD = Add()
SUB = Sub()
MULT = Mult()
DIV = Div()
BLEND = Blend()
CONSTANT = Constant()
VARIABLE = Variable()
IF_EXPR = ConditionalExpression()
SUBROUTINE = subroutine.SubroutineFunc()
COND_BRANCH = CondBranch()

