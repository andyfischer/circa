

from Circa.builders import function_builder
from Circa.core import (builtins, ca_function, ca_type)

class Feedback(function_builder.BaseFunction):
    name = 'feedback'
    inputTypes = [builtins.REFERENCE_TYPE, builtins.REFERENCE_TYPE]
    outputType = builtins.VOID_TYPE
    hasState = False
    pureFunction = False

    @staticmethod
    def evaluate(term):
        target = term.getInput(0)
        desired = term.getInput(1)

        ca_function.feedbackFunc(target.functionTerm)(target,desired)

def createFunctions(codeUnit):
    builtins.FEEDBACK_FUNC = function_builder.createFunction(codeUnit, Feedback)
