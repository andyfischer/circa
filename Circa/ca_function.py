import pdb

class Function(object):
   def __init__(self, inputs=None, output=None, pureFunction=True, isGenerator=False,
         hasState=False,
         name="", init=None, evaluate=None):
      if inputs is None: inputs = []

      self.inputTypes = inputs
      self.outputType = output
      self.trainingFunc = None
      self.hasBranch = False
      self.pureFunction = pureFunction
      self.hasState = hasState
      self.isGenerator = False
      self.name = name

      if init is not None:
         self.pythonInit = init
      
      if evaluate is not None:
         self.pythonEvaluate = evaluate

   # pythonInit is called once per term, right after the term is created
   def pythonInit(self, term):
      pass

   # This funciton is called whenever evaluation is needed. The function is
   # suppossed to take values out of 'term's inputs, and stick some result in
   # term.pythonValue.
   def pythonEvaluate(self, term):
      pass
 
def wrapPythonFunction(pythonFunc):
   """
   This function wraps a Python function so that it is suitable to be used
   as the 'pythonEvaluate' portion of an existing Function. The values of
   all the arguments of the Circa term are copied as arguments to the Python
   function, and the returned result of the Python function is used as the
   'pythonValue' of the Circa term.
   """

   def funcForCirca(term):
       term.pythonValue = pythonFunc(*map(lambda t:t.pythonValue, term.inputs))
   return funcForCirca

def createFunctionFromPython(func, **initOptions):
   """
   This function returns an instance of Function, with 'func' implanted as its
   evaluation function. 'initOptions' are passed to the constructor of Function.
   """
   circaFunc = Function(**initOptions)
   circaFunc.pythonEvaluate = wrapPythonFunction(func)
   return circaFunc


