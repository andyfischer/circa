#
# Defines Circa funcitons for arithmetic

import math

from Circa.common import (debug, function_builder)
from Circa.core import (builtins, ca_float, ca_function)

class Add(object):
    name = 'add'
    inputs = ['float','float']
    output = 'float'
    feedbackHandler = 'add-feedback'

    @staticmethod
    def evaluate(a,b):
        debug._assert(a is not None)
        return a + b

class AddFeedback(function_builder.BaseFunction):
    name = 'add-feedback'
    inputs = ['ref','ref']
    output = 'void'
    pure = False

    @staticmethod
    def evaluate(cxt):
        target = cxt.inputTerm(0)
        targetsValue = cxt.input(0)
        desired = cxt.input(1)
        delta = targetsValue - desired
        halfDelta = cxt.codeUnit().createConstant(builtins.FLOAT_TYPE)
        ca_float.setValue(halfDelta, delta / 2.0)

        cxt.codeUnit().createTerm(builtins.FEEDBACK_FUNC,
            inputs=[target.getInput(0), halfDelta])
        cxt.codeUnit().createTerm(builtins.FEEDBACK_FUNC,
            inputs=[target.getInput(1), halfDelta])

class Subtract(object):
    name = 'sub'
    inputs = ['float','float']
    output = 'float'

    @staticmethod
    def evaluate(a,b):
        return a - b

class Multiply(object):
    name = 'mult'
    inputs = ['float','float']
    output = 'float'

    @staticmethod
    def evaluate(a,b):
        return a * b

class Divide(object):
    name = 'div'
    inputs = ['float','float']
    output = 'float'

    @staticmethod
    def evaluate(a,b):
        return a / b

class Average(object):
    name = 'average'
    inputs = ['float']
    output = 'float'
    variableArgs = True

    @staticmethod
    def evaluate(*inputs):
        count = len(inputs)
        if count == 0:
            return 0

        sum = 0.0
        for i in range(count):
            sum += inputs[i]
        return sum/count

class SquareRoot(object):
    name = 'sqrt'
    inputs = ['float']
    output = 'float'

    @staticmethod
    def evaluate(a):
        return math.sqrt(a)



def createFunctions(codeUnit):
    function_builder.createFunction(codeUnit, AddFeedback)
    add = function_builder.importPythonFunction(codeUnit, Add)

    for functionDef in (Subtract,Multiply,Divide,Average,SquareRoot):
        function_builder.importPythonFunction(codeUnit, functionDef)

