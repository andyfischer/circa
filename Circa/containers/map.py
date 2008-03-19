
from Circa import ca_function

def mapGeneratorEvaluate(keyType, valueType):

   # Note: type checking should go somewhere huh?

   mapFuncObj = ca_function.Function()
   mapFuncObj.hashtable = {}

   def mapEvaluate(self, term):
      key = term.inputs[0]
      term.pythonValue = self.hashtable[key]
   mapFuncObj.pythonEvaluate = mapEvaluate


