
from Circa import (
  code
)


class SubroutineDefinition(object):
  def __init__(self, input_names=None):
    self.code_unit = code.CodeUnit()

    # Set up placeholders
    self.input_placeholders = []
    self.this_placeholder = terms.placeholder()

    if input_names is not None:
      map(self.createInput, input_names)


  def createInput(self, input_name):
    assert isinstance(input_name, str)

    new_term = terms.placeholder()
    self.input_placeholders.append(new_term)
    return new_term

