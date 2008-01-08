
import code_unit
import term


class SubroutineDefinition(object):
  def __init__(self):
    self.code_unit = code_unit.CodeUnit()

    # Set up placeholders
    self.input_placeholders = []
    self.this_placeholder = term.placeholder()

  def createInput(self, input_name):
    new_term = term.placeholder()
    return new_term

