import numerical_training

def fullUpdate(code):
  assert isinstance(code, code_unit.CodeUnit)

  for term in code.iterate():
    if term.trainingInfo:
      term.trainingInfo.update(term)




# Training types
NumericalDerived = numerical_training.NumericalDerivedInfo
NumericalSource = numerical_training.NumericalSourceInfo
