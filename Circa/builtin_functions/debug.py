import pdb

from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)

class Print(ca_function.BaseFunction):
  name = "print"
    
  def pythonEvaluate(self, term):
    print str(term.inputs[0])

class GetInput(ca_function.BaseFunction):
  name = "input"
  input = signature.empty()
  outputType = ca_types.STRING
  pureFunction = False

  def pythonEvaluate(self, term):
    term.pythonValue = raw_input("> ")

class Assert(ca_function.BaseFunction):
  name = "assert"
  input = signature.fixed(ca_types.BOOL)
  outputType = None
  pureFunction = False

  def pythonEvaluate(self, term):
    if not term.inputs[0].pythonValue:
      print "Assertion failure for term: " + str(term.globalID)
