
from Circa import (
   ca_function,
   python_bridge
)

def mapGeneratorEvaluate(keyType, valueType):

   mapFuncObj = ca_function.createFunction(inputs=[keyType], output=valueType)
   mapFuncObj.hashtable = {}

   def mapEvaluate(term):
      key = term.inputs[0]
      if key in mapFuncObj.hashtable:
         term.pythonValue = mapFuncObj.hashtable[key]
      else:
         term.pythonValue = None
   mapFuncObj.pythonEvaluate = mapEvaluate

   return mapFuncObj

