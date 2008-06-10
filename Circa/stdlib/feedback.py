
import pdb
from Circa.builders import function_builder
from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import debug

class Feedback(function_builder.BaseFunction):
    name = 'feedback'
    inputTypes = [builtins.REF_TYPE, builtins.REF_TYPE]
    outputType = builtins.VOID_TYPE
    hasState = False
    pureFunction = False

    @staticmethod
    def evaluate(cxt):
        target = cxt.inputTerm(0)
        desired = cxt.inputTerm(1)

        feedbackAccumulator = ca_function.feedbackAccumulator(target.functionTerm)
        feedbackPropagator = ca_function.feedbackPropagator(target.functionTerm)

        # Todo: send desired through an accumulator.

        if feedbackPropagator is None:
            print "Error: no feedback propagator for", ca_function.name(target.functionTerm)
            return

        cxt.codeUnit().createTerm(feedbackPropagator, [target, desired])

def createFunctions(codeUnit):
    builtins.FEEDBACK_FUNC = function_builder.createFunction(codeUnit, Feedback)
