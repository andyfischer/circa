import numerical_training

def fullUpdate(code):
  assert isinstance(code, code_unit.CodeUnit)

  for term in code.iterate():
    if term.trainingInfo:
      term.trainingInfo.update(term)


class TrainingInfo(object):
  def update(self):
    raise Exception("Need to override this")


# Training types
NumericalDerived = numerical_training.NumericalDerivedInfo
NumericalSource = numerical_training.NumericalSourceInfo
