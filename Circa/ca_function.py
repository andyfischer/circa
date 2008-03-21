import pdb

class Function(object):
   def __init__(self, inputs=None, output=None, pureFunction=True):
      if inputs is None: inputs = []

      self.inputTypes = inputs
      self.outputType = output
      self.hasBranch = False
      self.pureFunction = pureFunction
      self.isGenerator = False

   # pythonFindExisting is called right before term creation would occur.
   # If this function returns not-None, then the builder will use the
   # result instead of creating a new term. This function may also be called
   # in situations where we are looking for an existing term but not 
   # planning on creating one.

   def pythonFindExisting(self, inputs):
      return None

   # pythonInit is called once per term, right after the term is created
   def pythonInit(self, term):
      pass

   def pythonEvaluate(self, term):
      pass
 

# Deprecated
def createFunction(inputs, output):
   f = Function(inputs=inputs, output=output)
   return f

def createUnknownFunction(name):
   f = Function()
   f.name = name
   return f

