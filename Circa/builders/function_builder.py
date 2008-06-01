
from Circa.core import builtins


# This class allows Python code to create a function in a convenient
# way. Then, the function 'createFunction' can be used to make a
# Circa-based function out of one of these classes.
class BaseFunction(object):
    pass
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
    term = codeUnit.createConstant(builtins.FUNCTION_TYPE)
    ca_function.setName(term, functionDef.name)
    ca_function.setInputTypes(term, functionDef.inputTypes)
    ca_function.setOutputType(term, functionDef.outputType)
    ca_function.setPureFunction(term, functionDef.pureFunction)
    ca_function.setHasState(term, functionDef.hasState)
    ca_function.setEvaluateFunc(term, functionDef.evaluate)
