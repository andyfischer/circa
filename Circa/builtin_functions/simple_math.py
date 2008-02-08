from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state,
  training
)


class Add(ca_function.BaseFunction):
  name = "add"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluateEmulated(self, term):
    term.pythonValue = float(term.inputs[0]) + float(term.inputs[1])

  def generateTraining(self, term, context, training_code):
    term.training_info.update()

    incoming_signal = context.incomingSignal(term)

    # count trainable inputs
    num_trainable_inputs = sum(map(lambda x: x > 0, term.training_info.input_blame))

    if num_trainable_inputs == 0: return

    for index in range(len(term.inputs)):

      # Normalize each input
      # (Todo: make sure there is an optimization that strips multiplication with 1)
      scalar = training_code.appendConstant(term.training_info.input_blame[index])
      product = training_code.append(MULT, inputs[incoming_signal, scalar])
      context.sendTrainingSignal(term.inputs[index], product)




class Sub(ca_function.BaseFunction):
  name = "sub"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluateEmulated(self, term):
    term.pythonValue = float(term.inputs[0]) - float(term.inputs[1])

class Mult(ca_function.BaseFunction):
  name = "mult"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluateEmulated(self, term):
    term.pythonValue = float(term.inputs[0]) * float(term.inputs[1])

class Div(ca_function.BaseFunction):
  name = "div"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluateEmulated(self, term):
    term.pythonValue = float(term.inputs[0]) / float(term.inputs[1])
    
class Blend(ca_function.BaseFunction):
  name = "blend"
  pureFunction = True
  inputType = ca_types.FLOAT
  outputType = ca_types.FLOAT
  trainingType = training.NumericalDerived

  def evaluateEmulated(self, term):
    blend_emulatedValue = float(term.inputs[2])
    term.pythonValue = float(term.inputs[0]) * (1 - blend_emulatedValue) + float(term.inputs[1]) * blend_emulatedValue


