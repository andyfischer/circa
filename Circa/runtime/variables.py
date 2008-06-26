
from Circa.common import function_builder
from Circa.core import (builtins, ca_variable, ca_function, ca_type)

def emptyFunction(term):
    pass

class VariableGenerator(object):
    name = '_variable-generator'
    pureFunction = True
    inputs = ['type']
    output = 'Function'
    meta = True

    @staticmethod
    def evaluate(cxt):
        type = cxt.inputTerm(0)

        ca_function.setOutputType(cxt.caller(), type)
        ca_function.setHasState(cxt.caller(), True)
        ca_function.setName(cxt.caller(), 'variable-' + ca_type.name(type))
        ca_function.setEvaluateFunc(cxt.caller(), Variable_evaluate)
        ca_function.setFeedbackPropagator(cxt.caller(), ASSIGN_FUNCTION)

def Variable_evaluate(cxt):
    # Copy state to output
    cxt.setResult(cxt.state())

class Assign(object):
    name = 'assign'
    pure = False
    inputs = ['ref','ref']
    inputNames = ['target', 'value']
    output = 'void'
    meta = True

    @staticmethod
    def evaluate(cxt):
        # Simple, copy 'desired' directly to 'target'
        target = cxt.inputTerm(0)
        ca_variable.setValue(target, cxt.input(1))
    

ASSIGN_FUNCTION = None

def createFunctions(codeUnit):
    builtins.VARIABLE_GENERATOR = (
        function_builder.importPythonFunction(codeUnit, VariableGenerator))

    global ASSIGN_FUNCTION
    ASSIGN_FUNCTION = function_builder.importPythonFunction(codeUnit, Assign)

