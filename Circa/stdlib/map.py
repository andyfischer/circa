
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
    def evaluate(term, keyType, valueType):
        ca_function.setName(term, 'map-' + keyType.name + '-to-'
                + valueType.name)
        ca_function.setInputTypes(term, [keyType])
        ca_function.setOutputType(term, [valueType])
        ca_function.setHasState(term, False)
        ca_function.setEvaluateFunc(term, evaluate_MapAccess)

def evaluate_MapAccess(term):
    term.cachedValue = term.functionTerm.state[term.getInput(0)]

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


