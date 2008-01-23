import ca_function
import ca_types
import training

class Add(ca_function.BaseFunction):
  name = "add"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluate(self, term):
    term.value = float(term.inputs[0]) + float(term.inputs[1])

class Sub(ca_function.BaseFunction):
  name = "sub"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluate(self, term):
    term.value = float(term.inputs[0]) - float(term.inputs[1])

class Mult(ca_function.BaseFunction):
  name = "mult"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluate(self, term):
    term.value = float(term.inputs[0]) * float(term.inputs[1])

class Div(ca_function.BaseFunction):
  name = "div"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluate(self, term):
    term.value = float(term.inputs[0]) / float(term.inputs[1])
    
class Blend(ca_function.BaseFunction):
  name = "blend"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluate(self, term):
    blend_value = float(term.inputs[2])
    term.value = float(term.inputs[0]) * (1 - blend_value) + float(term.inputs[1]) * blend_value


