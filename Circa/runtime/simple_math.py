#
# Defines Circa funcitons for arithmetic

from Circa.common import function_builder
from Circa.core import (builtins, ca_float, ca_function)

class Add(object):
    name = 'add'
    inputs = ['float','float']
    output = 'float'

    @staticmethod
    def evaluate(a,b):
        return a + b

class AddFeedback(function_builder.BaseFunction):
    name = '_add-feedback'
    inputTypes = [builtins.REF_TYPE, builtins.REF_TYPE]
    outputType = builtins.VOID_TYPE
    pureFunction = False

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


def createFunctions(codeUnit):
    add = function_builder.importPurePythonFunction(codeUnit, Add)
    ca_function.setFeedbackPropagator(add,
            function_builder.createFunction(codeUnit, AddFeedback))


    for functionDef in (Subtract,Multiply,Divide,Average):
        function_builder.importPurePythonFunction(codeUnit, functionDef)

