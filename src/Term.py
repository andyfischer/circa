

class Term(object):
  def __init__(m, initial_inputs=[]):

    m.function = None
    m.inputs = []
    m.training_info = None

    m.setInputs(initial_inputs)

  def setInputs(m, list):
    old_inputs = 



