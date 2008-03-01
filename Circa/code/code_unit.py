"""
This object contains a single 'unit' of code.

Generally, one unit of code corresponds with one subroutine definition, but the user is
free to do it differently.

All structure-changing operations should be performed on this object,
rather than on the terms themselves.
"""

import pdb

from Circa import (
  builtins,
  ca_function,
  common_errors,
  pythonTypes,
  terms
)

from Circa.utils import indent_printer

VERBOSE_DEBUGGING = False

class CodeUnit(object):
  def __init__(self):
    self.main_branch = []
    self.term_namespace = {}

  def getTerm(self, functionTerm, inputs, **term_options):

    existing_term = terms.findExisting(functionTerm, inputs)
    if existing_term:
      return existing_term

    return self.createTerm(functionTerm, inputs, **term_options)

  def createTerm(self, functionTerm, inputs=None, branch=None, name=None,
      initialValue=None, **term_options):

    # Create a new term
    new_term = terms.createTerm(functionTerm, code_unit=self, **term_options)
    assert new_term is not None

    # Add term to function's users
    functionTerm.users.add(new_term)

    if initialValue is not None:
      new_term.pythonValue = initialValue

    if inputs:
      # If they use any non-term args, convert them to constants
      self.setTermInputs(new_term, inputs)

    if name:
      self.setTermName(new_term, name)

    if branch is None: branch = self.main_branch
    branch.append(new_term)

    return new_term


  def createConstant(self, value, name=None, branch=None,
        source_token=None, type=None):

    if type is None and value is None:
      raise Exception("Either type or value needs to be not-None")

    if type is None:
      type = pythonTypes.typeOfPythonObj(value)

    if type is None:
      raise Exception("Couldn't find a type for value: " + str(value))

    # Get constant function for this type
    constFunc = terms.findExisting(builtins.CONST_FUNC, inputs=[type])

    # Create a constant function if it wasn't found
    if constFunc is None:
      funcValue = ca_function.createFunction(inputs=[], outputs=[type])
      constFunc = self.createTerm(builtins.CONST_FUNC, inputs=[type],
          initialValue=funcValue)
      constFunc.debugName = "constant-" + type.getSomeName()
      assert constFunc.pythonValue is not None

    # Look for an existing term
    existingTerm = terms.findExistingConstant(constFunc, value)

    term = None
    if existingTerm is None:
        term = self.createTerm(constFunc, initial_value=value, 
            branch=branch, source_token=source_token)
    else:
        term = existingTerm

    if name:
      self.setTermName(term, name)

    return term

  def setTermName(self, term, name, allow_rename=False):
    "Set a term's name"

    assert isinstance(name, str)

    if (not allow_rename) and (name in self.term_namespace):
      raise common_errors.UsageError("A term with name \""+str(name)+"\" already exists." +
                       " (Use 'allow_rename' if you want to allow this)")

    if name in self.term_namespace:
      self.term_namespace[name].givenName = None
      # TODO, currently the 'givenName' variable will not behave if a term is given
      # more than one name

    self.term_namespace[name] = term
    term.givenName = name

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
    if term.functionTerm.pythonValue.pureFunction:
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
    #if values.isConstant(term):
      #continue

    text = term.getSomeName() + ": " + term.functionTerm.getSomeName()

    if term.inputs:
      text += " ("
      text += ",".join(map(terms.Term.getSomeName, term.inputs))
      text += ")"
 
    printer.println(text)
 
    if term.branch:
       printer.indent()
       printTermsFormatted(term.branch, printer, term_names)
       printer.unindent()
