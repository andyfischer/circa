"""
This object contains a single 'unit' of code.

Generally, one unit of code corresponds with one subroutine definition, but the user is
free to do it differently.

All structure-changing operations should be performed on this object,
rather than on the terms themselves.
"""

import pdb

import term_utils
import term as term_module

from Circa import (
   builtins,
   ca_function,
   common_errors,
   debug,
   python_bridge
)

import term as _term
from Circa.utils import indent_printer

VERBOSE_DEBUGGING = False

class CodeUnit(object):
   def __init__(self):
      self.main_branch = []
      self.term_namespace = {}
 
   def getTerm(self, functionTerm, inputs, **term_options):

      # Make sure function and inputs are Terms
      debug.Assert(isinstance(functionTerm, _term.Term))
      for input in inputs:
         debug.Assert(isinstance(input, _term.Term))
      debug.Assert(functionTerm.pythonValue is not None)

      # First, check to reuse an existing term
      shouldTryToReuse = True

      # If the funciton has state, don't reuse
      if functionTerm.pythonValue.hasState:
         shouldTryToReuse = False

      if shouldTryToReuse:
         equivalentExisting = term_utils.findExisting(functionTerm, inputs)
         if equivalentExisting:
            return equivalentExisting
 
      return self.createTerm(functionTerm, inputs, **term_options)
 
   def createTerm(self, functionTerm, inputs=None, branch=None,
         name=None, sourceToken=None):

      debug.Assert(isinstance(functionTerm, _term.Term))
      debug.Assert(functionTerm.pythonValue is not None)
 
      # Create a new term
      term = term_module.Term()
      term.functionTerm = functionTerm
      term.sourceToken = sourceToken
      term.codeUnit = self
  
      # Check to create a branch
      if ca_function.hasBranch(functionTerm):
          term.branch = []
  
      # Add term to function's users
      functionTerm.users.add(term)
  
      if inputs:
         self.setTermInputs(term, inputs, suppressChangeEvent=True)
  
      if name:
         self.setTermName(term, name)

      # Initialize
      functionTerm.pythonValue.pythonInit(term)

      # Evaluate immediately (in some cases)
      if (ca_function.pureFunction(functionTerm)):
          term.pythonEvaluate()
  
      if branch is None: branch = self.main_branch
      branch.append(term)
  
      return term

   def createConstant(self, value=None, name=None, branch=None,
         sourceToken=None, valueType=None):
  
      if valueType is None and value is None:
         raise Exception("Either type or value needs to be not-None")
  
      if valueType is None:
         valueType = python_bridge.typeOfPythonObj(value)
  
      if valueType is None:
         raise Exception("Couldn't find a type for value: " + str(value))
  
      # Get constant function for this type
      constFunc = self.getTerm(builtins.CONST_FUNC_GENERATOR, inputs=[valueType])
  
      # Look for an existing term
      existingTerm = term_utils.findExistingConstant(constFunc, value)
  
      term = None
      if existingTerm is None:
          term = self.createTerm(constFunc, branch=branch, sourceToken=sourceToken)
          term.pythonValue = value
      else:
          term = existingTerm
  
      if name:
         self.setTermName(term, name)
  
      return term

   def createVariable(self, name=None, branch=None, sourceToken=None,
       valueType=None):

      debug.Assert(name is None or isinstance(name,str))
      debug.Assert(isinstance(valueType, _term.Term))

      # Get variable function for this type
      variableFunc = self.getTerm(builtins.VARIABLE_FUNC_GENERATOR, inputs=[valueType])

      return self.getTerm(variableFunc, inputs=[])
 
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
 
   def setTermInputs(self, target_term, new_inputs, suppressChangeEvent=False):
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
  
      if not suppressChangeEvent:
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
      if ca_function.pureFunction(term):
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

   def getTermLabel(term):
      if term.isConstant():
         return "[constant] " + str(term.pythonValue)
      else:
         return term.getSomeName()
   
   for term in branch:

    # Skip constants
    if term.isConstant():
       continue

    text = term.getSomeName() + ": " + term.functionTerm.getSomeName()

    if term.inputs:
       text += " ("
       text += ", ".join(map(getTermLabel, term.inputs))
       text += ")"
 
    printer.println(text)
 
    if term.branch:
       printer.indent()
       printTermsFormatted(term.branch, printer, term_names)
       printer.unindent()
