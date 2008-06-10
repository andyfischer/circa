#
# Defines Circa funcitons for arithmetic

from Circa.builders import function_builder
from Circa.core import (builtins, ca_float, ca_function)

class Add(function_builder.BaseFunction):
    name = 'add'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.INT_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        return cxt.input(0) + cxt.input(1)

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
        ca_float.setvalue(halfDelta, delta / 2.0)

        cxt.codeUnit().createTerm(builtins.FEEDBACK_FUNC,
            inputs=[target.getInput(0), halfDelta])
        cxt.codeUnit().createTerm(builtins.FEEDBACK_FUNC,
            inputs=[target.getInput(1), halfDelta])

class Subtract(function_builder.BaseFunction):
    name = 'sub'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.INT_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        return cxt.input(0) - cxt.input(1)

class Mult(function_builder.BaseFunction):
    name = 'mult'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.INT_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        return cxt.input(0) * cxt.input(1)

class Divide(function_builder.BaseFunction):
    name = 'div'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.INT_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        return cxt.input(0) / cxt.input(1)

class Average(function_builder.BaseFunction):
    name = 'average'
    inputTypes = [builtins.INT_TYPE]
    outputType = builtins.INT_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        if cxt.numInputs() == 0:
            return 0

        sum = 0
        for i in range(cxt.numInputs()):
            sum += cxt.input(i)
        return sum / cxt.numInputs()
    

functionDefs = [Subtract, Mult, Divide, Average]

def createFunctions(codeUnit):
    add = function_builder.createFunction(codeUnit, Add)
    ca_function.setFeedbackPropagator(add,
            function_builder.createFunction(codeUnit, AddFeedback))

    for functionDef in functionDefs:
        function_builder.createFunction(codeUnit, functionDef)

