
import code_unit, term

class SubroutineDefinition(object):
  def __init__(self, input_names=None):
    self.code_unit = code_unit.CodeUnit()

    # Set up placeholders
    self.input_placeholders = []
    self.this_placeholder = term.createPlaceholder()

    if input_names is not None:
      map(self.createInput, input_names)

  def createInput(self, input_name):
    # input_name is either a string or None
    new_term = term.createPlaceholder()
    self.input_placeholders.append(new_term)
    return new_term

def getSubroutineEvaluate(sub_def):
   def subroutineEvaluate():
      sub_def.code_unit.evaluate()
      

