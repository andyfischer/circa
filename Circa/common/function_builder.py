
from Circa.common import debug
from Circa.core import (builtins, ca_function, ca_type)
from Circa.core.term import Term


# This class allows Python code to create a function in a convenient
# way. Then, the function 'createFunction' can be used to make a
# Circa-based function out of one of these classes.
class BaseFunction(object):
    pureFunction = False
    hasState = False
    variableArgs = False
    inputs = []
    output = 'void'
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
    debug.assertType(functionDef.inputs, list, functionDef.__name__+'.inputs')
    debug.assertType(functionDef.output, str, functionDef.__name__+'.output')

    def findType(typeName):
        type = codeUnit.getNamed(typeName)
        if type is None:
            raise Exception("Type not found: " + typeName)
        return type

    term = codeUnit.createConstant(builtins.FUNCTION_TYPE)
    ca_function.setName(term, functionDef.name)
    ca_function.setInputTypes(term, map(findType, functionDef.inputs))
    ca_function.setOutputType(term, findType(functionDef.output))
    ca_function.setPureFunction(term, functionDef.pureFunction)
    ca_function.setHasState(term, functionDef.hasState)
    ca_function.setVariableArgs(term, functionDef.variableArgs)
    ca_function.setInitializeFunc(term, functionDef.initialize)
    ca_function.setEvaluateFunc(term, functionDef.evaluate)
    codeUnit.bindName(term, functionDef.name)
    return term

def importPythonFunction(codeUnit, pythonClass, instanceBased = False):
    """
    Create a Circa function out of the specially-formed Python class.
    This class must have the following fields defined:

      name: string
      inputs: a list of strings representing Circa types
      output: a string representing a Circa type
      evaluate: a static function
    """

    debug._assert(isinstance(pythonClass, type))
    debug.assertType(pythonClass.inputs, list, 'pythonClass.inputs')

    def findType(typeName):
        type = codeUnit.getNamed(typeName)
        if type is None:
            raise Exception("Type not found: " + typeName)
        return type

    functionTerm = codeUnit.createConstant(builtins.FUNCTION_TYPE)
    ca_function.setName(functionTerm, pythonClass.name)
    ca_function.setInputTypes(functionTerm, map(findType, pythonClass.inputs))
    ca_function.setOutputType(functionTerm, findType(pythonClass.output))
    ca_function.setHasState(functionTerm, instanceBased)

    if not instanceBased:
        def initializeFunc(cxt):
            pass
        def evaluateFunc(cxt):
            try:
                inputs = [cxt.input(n) for n in range(cxt.numInputs())]
                result = pythonClass.evaluate(*inputs)
                cxt.setResult(result)
            except Exception, e:
                print "An internal error occured in " + pythonClass.name
    else:
        def initializeFunc(cxt):
            cxt.caller().state = pythonClass()
            cxt.caller().state.initialize()

        def evaluateFunc(cxt):
            inputs = [cxt.input(n) for n in range(cxt.numInputs())]
            result = cxt.caller().state.evaluate(*inputs)
            cxt.setResult(result)

    ca_function.setInitializeFunc(functionTerm, initializeFunc)
    ca_function.setEvaluateFunc(functionTerm, evaluateFunc)

    pureFunction = True
    if hasattr(pythonClass, 'pure'):
        pureFunction = pythonClass.pure
    ca_function.setPureFunction(functionTerm, pureFunction)

    variableArgs = False
    if hasattr(pythonClass, 'variableArgs'):
        variableArgs = pythonClass.variableArgs
    ca_function.setVariableArgs(functionTerm, variableArgs)

    # Feedback handler
    if hasattr(pythonClass, 'feedbackHandler'):
        fhName = pythonClass.feedbackHandler
        fhTerm = codeUnit.getNamed(fhName)
        if fhTerm is None:
            raise Exception("Couldn't find term: " + fhName)
        ca_function.setFeedbackPropagator(functionTerm, fhTerm)

    codeUnit.bindName(functionTerm, pythonClass.name)
    return functionTerm

def importPythonType(codeUnit, pythonClass):
    """
    Import the given Python class as a Circa type.
    This class must define the following fields:
      name
      toShortString
      iterateInnerTerms
    """

    debug._assert(isinstance(pythonClass, type))

    typeTerm = codeUnit.createConstant(builtins.TYPE_TYPE)

    ca_type.setName(typeTerm, pythonClass.name)

    def initialize(term):
        term.cachedValue = pythonClass()

    def toShortString(term):
        return term.cachedValue.toShortString()

    def iterateInnerTerms(term):
        return term.cachedValue.iterateInnerTerms()

    ca_type.setInitialize(typeTerm, initialize)
    ca_type.setToShortString(typeTerm, toShortString)
    ca_type.setIterateInnerTerms(typeTerm, iterateInnerTerms)

    codeUnit.bindName(typeTerm, pythonClass.name)

    return typeTerm

