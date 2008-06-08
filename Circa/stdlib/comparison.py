
# Defines Circa functions for numeric comparison and equality checking

from Circa.builders import function_builder
from Circa.core import builtins

class Equals(function_builder.BaseFunction):
    name = 'equals'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.BOOL_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(context):
        return context.input(0) == context.input(1)

class NotEquals(function_builder.BaseFunction):
    name = 'not_equals'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.BOOL_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(context):
        return context.input(0) != context.input(1)

functionDefs = [Equals, NotEquals]

def createFunctions(codeUnit):
    for functionDef in functionDefs:
        function_builder.createFunction(codeUnit, functionDef)


