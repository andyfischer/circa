
import ca_function

class And(ca_function.BaseFunction):
  def init(self):
    self.name = "and"
    self.pureFunction = True
    self.inputType = bool
    self.outputType = bool

  def evaluate(self, term):
    for input in term.inputs:
      if not input.value:
        term.value = False
        return
    term.value = True

class Or(ca_function.BaseFunction):
  def init(self):
    self.name = "or"
    self.pureFunction = True
    self.inputType = bool
    self.outputType = bool

  def evaluate(self, term):
    for input in term.inputs:
      if input.value:
        term.value = True
        return
    term.value = False

class ConditionalExpression(ca_function.BaseFunction):
  def init(self):
    self.name = "if"
    self.pureFunction = True

  def evaluate(self, term):
    if term.inputs[0].value:
      term.value = term.inputs[1].value
    else:
      term.value = term.inputs[2].value

