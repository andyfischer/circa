import ca_function

class Add(ca_function.BaseFunction):
  def init(self):
    self.name = "add"
    self.pureFunction = True
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    term.value = float(term.inputs[0]) + float(term.inputs[1])

class Sub(ca_function.BaseFunction):
  def init(self):
    self.name = "sub"
    self.pureFunction = True
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    term.value = float(term.inputs[0]) - float(term.inputs[1])

class Mult(ca_function.BaseFunction):
  def init(self):
    self.name = "mult"
    self.pureFunction = True
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    term.value = float(term.inputs[0]) * float(term.inputs[1])

class Div(ca_function.BaseFunction):
  def init(self):
    self.name = "div"
    self.pureFunction = True
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    term.value = float(term.inputs[0]) / float(term.inputs[1])
    
class Blend(ca_function.BaseFunction):
  def init(self):
    self.name = "blend"
    self.pureFunction = True
    self.inputType = float
    self.outputType = float

  def evaluate(self, term):
    blend_value = float(term.inputs[2])
    term.value = float(term.inputs[0]) * (1 - blend_value) + float(term.inputs[1]) * blend_value


