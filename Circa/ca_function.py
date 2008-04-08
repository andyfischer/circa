import pdb

from Circa import (
      python_bridge
)

class _Function(object):
   def __init__(self):

      self.inputTypes = []
      self.outputType = None
      self.feedbackFunc = None
      self.hasBranch = False
      self.pureFunction = True
      self.hasState = False
      self.name = ""

   # pythonInit is called once per term, right after the term is created
   def pythonInit(self, term):
      pass

   # This function is called whenever evaluation is needed. The function is
   # suppossed to take values out of 'term's inputs, and stick some result in
   # term.pythonValue.
   def pythonEvaluate(self, term):
      pass

def setValue(term, inputs=None, output=None, pureFunction=None, 
      hasState=None, name=None, initFunc=None, evaluateFunc=None, feedbackFunc=None):

   # Make sure term has a _Function object
   if term.pythonValue is None:
      term.pythonValue = _Function()

   if inputs is not None: term.pythonValue.inputs = inputs
   if output is not None: term.pythonValue.outputType = output
   if pureFunction is not None: term.pythonValue.pureFunction = pureFunction
   if hasState is not None: term.pythonValue.hasState = hasState
   if name is not None: term.pythonValue.name = name
   if initFunc is not None: term.pythonValue.pythonInit = initFunc
   if evaluateFunc is not None: term.pythonValue.pythonEvaluate = evaluateFunc
   if feedbackFunc is not None: term.pythonValue.feedbackFunc = feedbackFunc
 
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
def getName(term):
   return term.pythonValue.name
def getInitFunc(term):
   return term.pythonValue.pythonInit
def getEvaluateFunc(term):
   return term.pythonValue.pythonEvaluate
def feedbackFunc(term):
   return term.pythonValue.feedbackFunc

def createFunctionFromPython(func, **initOptions):
   """
   This function returns an instance of Function, with 'func' implanted as its
   evaluation function. 'initOptions' are passed to the constructor of Function.
   """
   circaFunc = _Function(**initOptions)
   circaFunc.pythonEvaluate = python_bridge.wrapPythonFunction(func)
   return circaFunc

def createMetaFunctionFromPython(func, **initOptions):
   """
   This function returns an instance of Function, with 'func' implanted as its
   evaluation function. Unlike createFunctionFromPython, this does not pass 'func'
   through wrapPythonFunction. So, 'func' should accept a single term as input, and
   return nothing.
   """
   circaFunc = _Function(**initOptions)
   circaFunc.pythonEvaluate = func
   return circaFunc

