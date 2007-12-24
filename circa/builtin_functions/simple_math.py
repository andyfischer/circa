from basefunction import BaseFunction

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

ADD = Add()
SUB = Sub()
MULT = Mult()
DIV = Div()
BLEND = Blend()

