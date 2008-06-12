
# Defines various binary functions

from Circa.common import function_builder
from Circa.core import builtins

class And(function_builder.BaseFunction):
    name = "and"
    inputTypes = [builtins.BOOL_TYPE, builtins.BOOL_TYPE]
    outputType = builtins.BOOL_TYPE

    @staticmethod
    def evaluate(cxt):
        cxt.setResult( cxt.input(0) and cxt.input(1) )

class Or(function_builder.BaseFunction):
    name = "or"
    inputTypes = [builtins.BOOL_TYPE, builtins.BOOL_TYPE]
    outputType = builtins.BOOL_TYPE

    @staticmethod
    def evaluate(cxt):
        cxt.setResult( cxt.input(0) or cxt.input(1) )

functionDefs = [And, Or]

def createFunctions(codeUnit):
    for functionDef in functionDefs:
        function_builder.createFunction(codeUnit, functionDef)

