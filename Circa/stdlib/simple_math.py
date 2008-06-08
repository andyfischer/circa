#
# Defines Circa funcitons for arithmetic

from Circa.builders import function_builder
from Circa.core import builtins

class Add(function_builder.BaseFunction):
    name = 'add'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.INT_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        return cxt.input(0) + cxt.input(1)

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
    

functionDefs = [Add, Subtract, Mult, Divide]

def createFunctions(codeUnit):
    for functionDef in functionDefs:
        function_builder.createFunction(codeUnit, functionDef)

