import pdb

from Circa import (
   builtins,
   ca_function
)

nextGlobalID = 1

class Term(object):
   def __init__(self):
      "Use 'createTerm' instead of calling this constructor"
 
      self.inputs = []
      self.functionTerm = None
      self.users = set()

      # The output value of this term
      self.pythonValue = None

      # True if 'pythonValue' is available. False if this function has not
      # been evaluated, or cannot be evaluated.
      self.outputAvail = False

      self.state = None
      self.codeUnit = None
      self.branch = None
      self.givenName = None
      self.debugName = None
 
      global nextGlobalID
      self.globalID = nextGlobalID
      nextGlobalID += 1
 
   def getType(self):
      "Returns this term's output type"
      return ca_function.outputType(self.functionTerm)
 
   def getFunction(self):
      "Returns this term's Function"
      return self.functionTerm.pythonValue
 
   def getSomeName(self):
      """
      Returns some unique identifier. There are a few values we may use here.
      No guarantees are made as to the format.
      """
      if self.givenName is not None:
         return self.givenName
      elif self.debugName is not None:
         return self.debugName
      elif self.getType() is builtins.FUNC_TYPE:
         return self.pythonValue.name
      else:
         return self.getUniqueName()

   def getUniqueName(self):
      return 't' + str(self.globalID)
 
   def inputsContain(self, term):
      "Returns True if our inputs contain the term"
      for input in self.inputs:
         if input == term: return True
      return false
 
   def evaluate(self):
      self.getFunction().pythonEvaluate(self)

   # Obsolete alias
   pythonEvaluate = evaluate
 
   def printExtended(self, printer):
      printer.prints("%i: %s" % (self.globalID, self.functionTerm.pythonValue.name))
  
      if self.pythonValue:
         printer.prints(" " + str(self.pythonValue))
  
      printer.prints(" [")
  
      first_item = True
      for input in self.inputs:
         if not first_item: printer.prints(",")
         printer.prints(input.globalID)
         first_item = False
      printer.prints("]")
  
      printer.println()
  
      if self.state:
         label_branches = len(self.state.branches) > 1
         branch_index = -1
   
         for branch in self.state.branches:
            branch_index += 1
    
            # (maybe) label the branch index
            if label_branches:
               printer.indent(1)
               printer.println('Branch %i:' % branch_index)
               printer.indent(1)
            else:
               printer.indent(2)
    
            # print inner terms
            if branch.terms:
               for term in branch.terms:
                  term.printExtended(printer)
            else:
               printer.println("(empty)")
    
            printer.unindent(2)
 
   def iterate(self, forwards=True):
      yield self
      if self.branch:
         for term in self.branch:
            yield term
  
   def equals(self, term):
      assert isinstance(term, Term)
      return self.pythonValue == term.pythonValue
 
   def isConstant(self):
      return self.functionTerm.isConstantFunction()
 
   def isConstantFunction(self):
      return self.functionTerm is builtins.CONST_FUNC_GENERATOR
 
   # value accessors
   def __int__(self):
      try: return int(self.pythonValue)
      except: return 0
 
   def __float__(self):
      try: return float(self.pythonValue)
      except: return 0.0
 
   def __str__(self):
      try: return str(self.pythonValue)
      except: return ""
 
   # member accessor
   def __getitem__(self, name):
      return self.state.getLocal(name)


def createPlaceholder():
    term = Term()
    term.debugName = "Placeholder"
    return term

