
import term


class SubroutineDefinition(object):
  def __init__(self):
    self.all_terms = []
    self.term_namespace = {}

    # Set up placeholders
    self.input_placeholders = []
    self.this_placeholder = term.placeholder()

  def addInput(self, input_name):
    new_term = term.placeholder()

  def addTerm(self, term, name=None):
    self.all_terms.append(term)
    self.setTermName(term, name)

  def setTermName(term, name, allow_rename=False):
    if not allow_rename and name in self.term_namespace:
      raise UsageError("A term with name \""+str(name)+"\" already exists." +
                       "(Use 'allow_rename' if you want to allow this)"

    self.term_namespace[name] = term
