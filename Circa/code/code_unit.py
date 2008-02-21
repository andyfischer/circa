"""
This object contains a single 'unit' of code.

Generally, one unit of code corresponds with one subroutine definition, but the user is
free to do it differently.

All structure-changing operations should be performed on this object,
rather than on the terms themselves.
"""

from Circa import (
  ca_types,
  terms,
  common_errors,
  values
)

from Circa.utils import indent_printer

VERBOSE_DEBUGGING = False

class CodeUnit(object):
  def __init__(self):
    self.main_branch = []
    self.term_namespace = {}

  def createTerm(self, function, inputs=None, branch=None, **term_options):

    # Check to use an equivalent existing term
    existing_term = terms.findExisting(function, inputs)
    if existing_term:
      return existing_term

    # Create a new term
    new_term = terms.Term(function, code_unit=self, **term_options)

    if inputs:
      # If they use any non-term args, convert them to constants
      inputs = map(terms.wrapNonTerm, inputs)
      self.setTermInputs(new_term, inputs)

    if branch is None: branch = self.main_branch
    branch.append(new_term)

    return new_term

  def createConstant(self, value=None, name=None, branch=None, source_token=None):
    type = ca_types.getTypeOfPythonObj(value)
    term = self.createTerm(values.constFunctionFromType(type), initial_value=value, 
        branch=branch, source_token=source_token)

    if name:
      self.setTermName(term, name)

    return term

  def setTermName(self, term, name, allow_rename=False):
    "Set a term's name"

    assert isinstance(name, str)

    if (not allow_rename) and (name in self.term_namespace):
      raise common_errors.UsageError("A term with name \""+str(name)+"\" already exists." +
                       " (Use 'allow_rename' if you want to allow this)")

    self.term_namespace[name] = term

  def getNamedTerm(self, name):
    if name not in self.term_namespace: return None
    return self.term_namespace[name]

  def setTermInputs(self, target_term, new_inputs):
    "Assigns the term's inputs"

    old_inputs = target_term.inputs
    target_term.inputs = new_inputs

    # find which terms were just added
    newly_added = set(new_inputs) - set(old_inputs)

    # find which terms were just removed
    newly_removed = set(old_inputs) - set(new_inputs)

    # add ourselves to new user lists
    for t in newly_added:
      t.users.add(target_term)

    # remove ourselves from old user lists
    for t in newly_removed:
      t.users.remove(target_term)

    self.onInputsChanged(target_term)

  def appendToInput(self, target_term, new_input):
    "Append a term to the inputs of 'target_term'"

    # check if this input is newly added
    is_newly_added = new_input not in target_term.inputs

    target_term.inputs.append(new_input)

    # add to new user list
    if is_newly_added:
      target_term.users.add(new_input)

    self.onInputsChanged(target_term)
  
# Change events
  def onInputsChanged(self, term):
    # if this is a pure function then re-evaluate it
    if term.function.pureFunction:
      term.pythonEvaluate()


  def evaluate(self):
    if VERBOSE_DEBUGGING: print "code_unit.evaluate"

    for term in self.main_branch:
      if VERBOSE_DEBUGGING: print "Calling evaluate on " + str(term)
      term.pythonEvaluate()
 
  def printTerms(self):
    term_names = {}
 
    for (name,term) in self.term_namespace.items():
       term_names[term] = name
 
    printTermsFormatted(self.main_branch, indent_printer.IndentPrinter(), term_names)
  
  __getitem__ = getNamedTerm
  
  def iterateTerms(self):
    for term in self.main_branch:
      for t in term.iterate():
        yield t


def printTermsFormatted(branch, printer, term_names):
  for term in branch:

    # Skip constants
    if values.isConstant(term):
      continue

    if term in term_names:
      name = term_names[term]
    else:
      name = str(term.globalID)
    text = name + ": " + term.function.name

    if term.inputs:
      text += " ("

      def getTermLabel(term):
          if term in term_names:
             return term_names[term]
  
          # For constant terms, just write their value
          elif values.isConstant(term):
             return str(term.pythonValue)
  
          else:
             return 't:' + str(term.globalID)
 
 
      text += ",".join(map(getTermLabel, term.inputs))
      text += ")"
 
    printer.println(text)
 
    if term.branch:
       printer.indent()
       printTermsFormatted(term.branch, printer, term_names)
       printer.unindent()
