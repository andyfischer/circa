
import os, pdb
from Circa.core import (branch, builtins, ca_function, ca_type)
from Circa.common import (debug, function_builder)

class Feedback(object):
    name = 'apply-feedback'
    inputs = ['ref','ref']
    output = 'void'
    hasState = False
    pure = False
    instanceBased = True
    meta = True

    def initialize(self,cxt):
        # Create a branch
        self.branch = branch.Branch(cxt.caller())

    def evaluate(self,cxt):
        target = cxt.inputTerm(0)
        desired = cxt.inputTerm(1)
        
        # Delete anything currently in our branch
        self.branch.clear()

        feedbackAccumulator = ca_function.feedbackAccumulator(target.functionTerm)
        feedbackPropagator = ca_function.feedbackPropagator(target.functionTerm)

        # Todo: send desired through an accumulator.

        if feedbackPropagator is None:
            print "Error: no feedback propagator for", ca_function.name(target.functionTerm)
            return

        cxt.codeUnit().createTerm(feedbackPropagator, [target, desired],
                branch = self.branch)

        # Execute
        for term in self.branch:
            term.execute()

def createFunctions(codeUnit):
    builtins.FEEDBACK_FUNC = function_builder.importPythonFunction(codeUnit, Feedback)
