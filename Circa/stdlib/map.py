
import pdb

from Circa.builders import function_builder
from Circa.core import (builtins, ca_function, ca_type)

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
    def evaluate(term, in0, in1):
        keyType = term.getInput(0)
        valueType = term.getInput(1)
        ca_function.setName(term, 'map-' + 
                ca_type.name(keyType) + '-to-'
                + ca_type.name(valueType))
        ca_function.setInputTypes(term, [keyType])
        ca_function.setOutputType(term, valueType)
        ca_function.setHasState(term, False)
        ca_function.setPureFunction(term, True)
        ca_function.setEvaluateFunc(term, MapAccess_evaluate)
        ca_function.setFeedbackFunc(term, MapAccess_feedback)

def MapAccess_evaluate(term):
    hashtable = term.functionTerm.state
    key = term.getInput(0).cachedValue
    try:
        term.cachedValue = hashtable[key]
    except KeyError:
        term.cachedValue = None

def MapAccess_feedback(target, desired):
    # Goal here is to change target's function so that it outputs desired
    hashtable = target.functionTerm.state
    key = target.getInput(0).cachedValue
    hashtable[key] = desired.cachedValue

def createFunctions(codeUnit):
    function_builder.createFunction(codeUnit, MapConstructor)

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


