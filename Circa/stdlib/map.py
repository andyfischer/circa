
import pdb

from Circa.builders import function_builder
from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import debug

# Create Map functions
class MapConstructor(function_builder.BaseFunction):
    name = 'map'
    inputTypes = [builtins.TYPE_TYPE, builtins.TYPE_TYPE]
    outputType = builtins.FUNCTION_TYPE
    hasState = True
    pureFunction = True

    @staticmethod
    def initialize(term):
        term.state = {}

    @staticmethod
    def evaluate(cxt):
        keyType = cxt.inputTerm(0)
        valueType = cxt.inputTerm(1)
        ca_function.setName(cxt.caller(), 'map-' + 
                ca_type.name(keyType) + '-to-'
                + ca_type.name(valueType))
        ca_function.setInputTypes(cxt.caller(), [keyType])
        ca_function.setOutputType(cxt.caller(), valueType)
        ca_function.setHasState(cxt.caller(), False)
        ca_function.setPureFunction(cxt.caller(), True)
        ca_function.setEvaluateFunc(cxt.caller(), MapAccess_evaluate)
        ca_function.setFeedbackFunc(cxt.caller(), MAP_FEEDBACK)

def MapAccess_evaluate(cxt):
    hashtable = cxt.caller().functionTerm.state
    key = cxt.input(0)
    try:
        cxt.caller().cachedValue = hashtable[key]
    except KeyError:
        cxt.caller().cachedValue = None

class MapFeedback(function_builder.BaseFunction):
    name = 'map-feedback'
    inputTypes = [builtins.FUNCTION_TYPE, builtins.REFERENCE_TYPE]
    outputType = builtins.VOID_TYPE
    hasState = False
    pureFunction = False

    @staticmethod
    def evaluate(context):
        target = context.inputTerm(0)
        desired = context.input(1)

        # Goal here is to change target's function so that it outputs desired
        hashtable = target.functionTerm.state
        key = target.getInput(0).cachedValue
        hashtable[key] = desired
        target.functionTerm.needsUpdate = True

MAP_FEEDBACK = None

def createFunctions(codeUnit):
    function_builder.createFunction(codeUnit, MapConstructor)

    global MAP_FEEDBACK
    MAP_FEEDBACK = function_builder.createFunction(codeUnit, MapFeedback)

"""
m = map(string,string)
or
m = Map(string,string)

m('a') := 'b'
print m('a')

map-constructor:
    hasState = True
    inputTypes = [builtins.TYPE_TYPE, builtins.TYPE_TYPE]
    outputType = [builtins.FUNC_TYPE]
    state = current associations
    evaluate = return a Function where:
        hasState = False
        inputTypes = (first type)
        outputType = (second type)
        evaluate:
            in functionTerm.state, find association of input0 and return it
        feedback:
            in functionTerm.state, bind input0 to feedback0
"""


