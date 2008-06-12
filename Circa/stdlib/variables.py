
from Circa.common import function_builder
from Circa.core import (builtins, ca_function, ca_type)

def emptyFunction(term):
    pass

class VariableGenerator(function_builder.BaseFunction):
    name = '_variable-generator'
    pureFunction = True
    inputTypes = [builtins.TYPE_TYPE]
    outputType = builtins.FUNCTION_TYPE

    @staticmethod
    def evaluate(cxt):
        type = cxt.inputTerm(0)

        ca_function.setOutputType(cxt.caller(), type)
        ca_function.setHasState(cxt.caller(), True)
        ca_function.setName(cxt.caller(), 'variable-' + ca_type.name(type))
        ca_function.setEvaluateFunc(cxt.caller(), emptyFunction)

def createFunctions(codeUnit):
    builtins.VARIABLE_GENERATOR = (
        function_builder.createFunction(codeUnit, VariableGenerator))
