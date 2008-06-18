
from Circa.common import debug
from Circa.core import (builtins, ca_function)
from Circa.core.term import Term


# This class allows Python code to create a function in a convenient
# way. Then, the function 'createFunction' can be used to make a
# Circa-based function out of one of these classes.
class BaseFunction(object):
    pureFunction = False
    hasState = False
    variableArgs = False
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
    ca_function.setVariableArgs(term, functionDef.variableArgs)
    ca_function.setInitializeFunc(term, functionDef.initialize)
    ca_function.setEvaluateFunc(term, functionDef.evaluate)
    codeUnit.bindName(term, functionDef.name)
    return term

def importPurePythonFunction(codeUnit, pythonClass):
    """
    Create a Circa function out of the specially-formed Python class.
    This class must have the following fields defined:

      name: string
      inputs: a list of strings representing Circa types
      output: a string representing a Circa type
      evaluate: a static function
    """

    debug._assert(isinstance(pythonClass, type))

    def initializeFunc(cxt):
        pass
    def evaluateFunc(cxt):
        inputs = [cxt.input(n) for n in range(cxt.numInputs())]
        result = pythonClass.evaluate(*inputs)
        cxt.setResult(result)

    functionTerm = codeUnit.createConstant(builtins.FUNCTION_TYPE)
    ca_function.setName(functionTerm, pythonClass.name)

    def findType(typeName):
        type = codeUnit.getNamed(typeName)
        if type is None:
            raise Exception("Type not found: " + typeName)
        return type

    ca_function.setInputTypes(functionTerm, map(findType, pythonClass.inputs))
    ca_function.setOutputType(functionTerm, findType(pythonClass.output))
    ca_function.setPureFunction(functionTerm, True)
    ca_function.setHasState(functionTerm, False)
    ca_function.setInitializeFunc(functionTerm, initializeFunc)
    ca_function.setEvaluateFunc(functionTerm, evaluateFunc)
    codeUnit.bindName(functionTerm, pythonClass.name)
    return functionTerm

def importStatefulPythonFunction(codeUnit, pythonClass):
    debug._assert(isinstance(pythonClass, type))

    def initializeFunc(cxt):
        cxt.caller().state = pythonClass()

    def evaluateFunc(cxt):
        inputs = [cxt.input(n) for n in range(cxt.numInputs())]
        result = cxt.caller().state(*inputs)
        cxt.setResult(result)

    functionTerm = codeUnit.createConstant(builtins.FUNCTION_TYPE)
    ca_function.setName(functionTerm, pythonClass.name)
    
    def findType(typeName):
        type = codeUnit.getNamed(typeName)
        if type is None:
            raise Exception("Type not found: " + typeName)
        return type

    ca_function.setInputTypes(functionTerm, map(findType, pythonClass.inputs))
    ca_function.setOutputType(functionTerm, findType(pythonClass.output))
    ca_function.setPureFunction(functionTerm, False)
    ca_function.setHasState(functionTerm, True)
    ca_function.setInitializeFunc(functionTerm, initializeFunc)
    ca_function.setEvaluateFunc(functionTerm, evaluateFunc)
    codeUnit.bindName(functionTerm, pythonClass.name)
    return functionTerm
