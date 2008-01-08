"""
This object contains a single 'unit' of code.

Generally, one unit of code corresponds with one subroutine definition, but the user is
free to do it differently.

All structure-changing operations should be performed on this object
(rather than on the terms. (which is how things were done before)
"""

import term
from common_errors import UsageError

class CodeUnit(object):
  def __init__(self):
    self.all_terms = []
    self.main_branch = []
    self.term_namespace = {}

  def addTerm(self, term, name=None):
    "Add a new term"
    self.all_terms.append(term)
    if name:
      self.setTermName(term, name)

  def setTermName(self, term, name, allow_rename=False):
    assert isinstance(name, str)

    if (not allow_rename) and (name in self.term_namespace):
      raise UsageError("A term with name \""+str(name)+"\" already exists." +
                       " (Use 'allow_rename' if you want to allow this)")

    self.term_namespace[name] = term


  def setTermInputs(self, target_term, new_inputs):
    "Assigns the term's inputs"

    old_inputs = target_term.inputs
    target_term.inputs = new_inputs

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

  def getNamedTerm(self, name):
    return self.term_namespace[name]

  __getitem__ = getNamedTerm

