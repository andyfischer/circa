
def fullUpdate(code):
  assert isinstance(code, code_unit.CodeUnit)

  

def updateTrainingInfo(self, term):

  # check term's inputs for any training info
  should_have = False
  for input in term.inputs:
    if not input.training_info:
      continue

    if isinstance(input.training_info, NumericalTrainingInfo):
      should_have = True
      break

  # if this term shouldn't have numerical training then stop
  if not should_have:
    return

  # check if we need to create it
  if should_have and not term.training_info:
    term.training_info = NumericalTrainingInfo()

  # update


class NumericalTraining(object):
  def __init__(self):
    self.input_blame = []
