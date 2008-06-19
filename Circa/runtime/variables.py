
from Circa.common import function_builder
from Circa.core import (builtins, ca_function, ca_type)

def emptyFunction(term):
    pass

class VariableGenerator(function_builder.BaseFunction):
    name = '_variable-generator'
    pureFunction = True
    inputs = ['type']
    output = 'Function'

    @staticmethod
    def evaluate(cxt):
        type = cxt.inputTerm(0)

        ca_function.setOutputType(cxt.caller(), type)
        ca_function.setHasState(cxt.caller(), True)
        ca_function.setName(cxt.caller(), 'variable-' + ca_type.name(type))
        ca_function.setEvaluateFunc(cxt.caller(), Variable_evaluate)
        ca_function.setFeedbackPropagator(cxt.caller(), VARIABLE_FEEDBACK)

def Variable_evaluate(cxt):
    # Copy state to output
    cxt.setResult(cxt.state())

class VariableFeedback(function_builder.BaseFunction):
    name = '_variable-feedback'
    pure = False
    inputs = ['ref','ref']
    output = 'void'

    @staticmethod
    def evaluate(cxt):
        # Simple, copy 'desired' directly to 'target'
        target = cxt.inputTerm(0)
        assignVariable(target, cxt.inputTerm(1))
    
def assignVariable(target, value):
    """
    Assign 'value' to target.
    target should be a Variable term, value should be a plain value.
    """
    target.state = value
    target.update()

VARIABLE_FEEDBACK = None

def createFunctions(codeUnit):
    builtins.VARIABLE_GENERATOR = (
        function_builder.createFunction(codeUnit, VariableGenerator))

    global VARIABLE_FEEDBACK
    VARIABLE_FEEDBACK = function_builder.createFunction(codeUnit, VariableFeedback)

