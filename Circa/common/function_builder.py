
import pdb,traceback
from Circa.common import debug
from Circa.core import (builtins, ca_function, ca_type)
from Circa.core.term import Term


def importPythonFunction(codeUnit, pythonClass, instanceBased = False):
    """
    Create a Circa function out of the specially-formed Python class.
    This class must have the following fields defined:

      name: string
      inputs: a list of strings representing Circa types
      output: a string representing a Circa type
      evaluate: a static function
      instanceBased: boolean. If true, this will create a stateful function,
        which will use an instance of the given class as the term's state.
        The evaluate method will be bound, so you can refer to member
        fields within it.
      meta: boolean. If true, the evaluate function will be passed a
        TermExecutionContext, allowing for access of various things.
        If false, the evaluate function will just receive the values
        of its inputs.
        
    """

    debug._assert(isinstance(pythonClass, type))
    debug.assertType(pythonClass.inputs, list)

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

    try:
        instanceBased = pythonClass.instanceBased
    except:
        pass

    try:
        meta = pythonClass.meta
    except:
        meta = False

    def initializeFunc(cxt):
        pass

    if instanceBased:
        def initializeFunc(cxt):
            cxt.caller().state = pythonClass()

            if meta and hasattr(pythonClass, 'initialize'):
                cxt.caller().state.initialize(cxt)

    if meta:
        if instanceBased:
            def evaluateFunc(cxt):
                cxt.state().evaluate(cxt)
        else:
            def evaluateFunc(cxt):
                pythonClass.evaluate(cxt)

    else:
        if not instanceBased:
            def evaluateFunc(cxt):
                try:
                    inputs = [cxt.input(n) for n in range(cxt.numInputs())]
                    result = pythonClass.evaluate(*inputs)
                    cxt.setResult(result)
                except Exception, e:
                    print "An internal error occured in " + pythonClass.name
                    traceback.print_exc()
                    pdb.set_trace()

        else:

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
      instanceBased
    """

    debug._assert(isinstance(pythonClass, type))

    typeTerm = codeUnit.createConstant(builtins.TYPE_TYPE)

    ca_type.setName(typeTerm, pythonClass.name)

    instanceBased = True
    try:
        instanceBased = pythonClass.instanceBased
    except AttributeError: pass


    if instanceBased:
        def toShortString(term):
            return term.cachedValue.toShortString()

        def iterateInnerTerms(term):
            return term.cachedValue.iterateInnerTerms()
    else:
        toShortString = pythonClass.toShortString

        iterateInnerTerms = lambda: []
        try: iterateInnerTerms = pythonClass.iterateInnerTerms
        except AttributeError: pass

    ca_type.setAllocateData(typeTerm, lambda: pythonClass())
    ca_type.setToShortString(typeTerm, toShortString)
    ca_type.setIterateInnerTerms(typeTerm, iterateInnerTerms)

    codeUnit.bindName(typeTerm, pythonClass.name)

    return typeTerm

