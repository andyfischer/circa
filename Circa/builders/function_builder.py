
from Circa.common import debug
from Circa.core import (builtins, ca_function)
from Circa.core.term import Term


# This class allows Python code to create a function in a convenient
# way. Then, the function 'createFunction' can be used to make a
# Circa-based function out of one of these classes.
class BaseFunction(object):
    pureFunction = False
    hasState = False
    inputTypes = []
    outputType = None
    initialize = None
    
    """
    Should have these members:
       pureFunction (bool)
       hasState (bool)
       name (string)
       inputTypes (list of Circa type objects)
       outputType (Circa type object)
       evaluate (function)
    """

def createFunction(codeUnit, functionDef):
    # Do some type checks
    debug.assertType(functionDef.inputTypes, list, functionDef.__name__+'.inputTypes')
    debug.assertType(functionDef.outputType, Term, functionDef.__name__+'.outputType')

    term = codeUnit.createConstant(builtins.FUNCTION_TYPE)
    ca_function.setName(term, functionDef.name)
    ca_function.setInputTypes(term, functionDef.inputTypes)
    ca_function.setOutputType(term, functionDef.outputType)
    ca_function.setPureFunction(term, functionDef.pureFunction)
    ca_function.setHasState(term, functionDef.hasState)
    ca_function.setInitializeFunc(term, functionDef.initialize)
    ca_function.setEvaluateFunc(term, functionDef.evaluate)
    codeUnit.bindName(term, functionDef.name)
    return term

def convertPythonFuncToCircaEvaluate(pythonFunc):
   def funcForCirca(term):
       inputs = [term]
       inputs.extend(map(lambda t:t.cachedValue, term.inputs))
       result = pythonFunc(*inputs)
       if result is not None:
           term.cachedValue = result
   return funcForCirca
