
import os, pdb
from Circa.core import (branch, builtins, ca_function, ca_type)
from Circa.common import (debug, function_builder)

class Feedback(function_builder.BaseFunction):
    name = 'apply-feedback'
    inputTypes = [builtins.REF_TYPE, builtins.REF_TYPE]
    outputType = builtins.VOID_TYPE
    hasState = False
    pureFunction = False

    @staticmethod
    def initialize(cxt):
        # Create a branch
        cxt.caller().branch = branch.Branch(cxt.caller())

    @staticmethod
    def evaluate(cxt):
        target = cxt.inputTerm(0)
        desired = cxt.inputTerm(1)
        
        # Delete anything currently in our branch
        cxt.branch().clear()

        feedbackAccumulator = ca_function.feedbackAccumulator(target.functionTerm)
        feedbackPropagator = ca_function.feedbackPropagator(target.functionTerm)

        # Todo: send desired through an accumulator.

        if feedbackPropagator is None:
            print "Error: no feedback propagator for", ca_function.name(target.functionTerm)
            return

        cxt.codeUnit().createTerm(feedbackPropagator, [target, desired],
                branch = cxt.branch())

        # Execute
        for term in cxt.branch():
            term.execute()

def createFunctions(codeUnit):
    builtins.FEEDBACK_FUNC = function_builder.createFunction(codeUnit, Feedback)
