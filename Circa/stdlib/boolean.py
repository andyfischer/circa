
# Defines various binary functions

from Circa.builders import function_builder
from Circa.core import builtins

class And(function_builder.BaseFunction):
    name = "and"
    inputTypes = [builtins.BOOL_TYPE, builtins.BOOL_TYPE]
    outputType = builtins.BOOL_TYPE

    @staticmethod
    def evaluate(a, b):
        return a and b

class Or(function_builder.BaseFunction):
    name = "or"
    inputTypes = [builtins.BOOL_TYPE, builtins.BOOL_TYPE]
    outputType = builtins.BOOL_TYPE

    @staticmethod
    def evaluate(a, b):
        return a or b

functionDefs = [And, Or]

def createFunctions(codeUnit):
    for functionDef in functionDefs:
        function_builder.createFunction(codeUnit, functionDef)

