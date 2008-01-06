
import term
import term_creation_options

class CodeUnit(object):
  def __init__(self):
    self.all_terms = []
    self.term_namespace = {}

  def appendNewTerm(self, term_creation_options):
    assert isinstance(term_creation_options, TermCreationOptions)

    # Create the term
    new_term = term.Term(term_creation_options)

    # Append it to our list
    self.all_terms.append(new_term)

    # Initialize inputs
    self.setTermInputs(new_term, term_creation_options.inputs)

  def setTermInputs(self, target_term, new_inputs):
    "Assigns the term's inputs"

    old_inputs = target_term.inputs

    # find which terms were just added
    newly_added = new_inputs - old_inputs

    # find which terms were just removed
    newly_removed = old_inputs - new_inputs

    if not newly_added and not newly_removed:
      return

    # add ourselves to new user lists
    for t in newly_added:
      t.users.add(target_term)

    # remove ourselves from old user lists
    for t in newly_removed:
      t.users.remove(target_term)

  def getOutputType(self, target_term):
    return target_term.outputType

  def getNamedTerm(self, name):
    return self.term_namespace[name]

  __getitem__ = getNamedTerm

