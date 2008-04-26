import pdb

from Circa import (
   debug,
   python_bridge
)

class _Function(object):
   def __init__(self):
      self.inputTypes = []
      self.outputType = None
      self.hasBranch = False
      self.pureFunction = True
      self.hasState = False
      self.feedbackAccumulateFunction = None
      self.name = "undefined"

      # pythonInit is called once per term, right after the term is created
      self.pythonInit = None

      # This function is called whenever evaluation is needed
      self.pythonEvaluate = None

      self.pythonHandleFeedback = None

   def __str__(self):
      return "<Function " + self.name + ">"

def setValue(term, inputs=None, output=None, pureFunction=None, 
      hasState=None, name=None, initFunc=None, evaluateFunc=None,
      feedbackFunc=None):

   # If 'term' doesn't have a _Function object, then create one
   if term.pythonValue is None:
      term.pythonValue = _Function()

   if inputs is not None: term.pythonValue.inputs = inputs
   if output is not None: term.pythonValue.outputType = output
   if pureFunction is not None: term.pythonValue.pureFunction = pureFunction
   if hasState is not None: term.pythonValue.hasState = hasState
   if name is not None: term.pythonValue.name = name
   if initFunc is not None: term.pythonValue.pythonInit = initFunc
   if evaluateFunc is not None: term.pythonValue.pythonEvaluate = evaluateFunc
   if feedbackFunc is not None: term.pythonValue.pythonHandleFeedback = feedbackFunc

def setFromPythonFunction(term, pythonFunc):
   # Make sure name is defined
   debug.Assert(pythonFunc.name)

   setValue(term, pythonFunc.inputs, pythonFunc.output, pythonFunc.pureFunction,
      pythonFunc.hasState, pythonFunc.name, pythonFunc.initialize, pythonFunc.evaluate,
      pythonFunc.handleFeedback)

def inputTypes(term):
   return term.pythonValue.inputTypes
def outputType(term):
   return term.pythonValue.outputType
def pureFunction(term):
   return term.pythonValue.pureFunction
def hasState(term):
   return term.pythonValue.hasState
def hasBranch(term):
   # Not implemented
   return False
def name(term):
   return term.pythonValue.name

def callInit(func, term):
   if func.pythonValue.pythonInit is not None:
      func.pythonValue.pythonInit(term)

def callEvaluate(func, term):
   if func.pythonValue.pythonEvaluate is not None:
      func.pythonValue.pythonEvaluate(term)

def getInitFunc(term):
   return term.pythonValue.pythonInit
def getEvaluateFunc(term):
   return term.pythonValue.pythonEvaluate
def feedbackAccumulateFunction(term):
   return term.pythonValue.feedbackAccumulateFunction
def handleFeedback(term):
   return term.pythonValue.pythonHandleFeedback
