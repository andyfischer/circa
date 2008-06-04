#
# Defines Circa funcitons for arithmetic

from Circa.builders import function_builder
from Circa.core import builtins

class Add(function_builder.BaseFunction):
    name = 'add'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.INT_TYPE

    @staticmethod
    def evaluate(term,a,b):
        return a + b

class Mult(function_builder.BaseFunction):
    name = 'mult'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.INT_TYPE

    @staticmethod
    def evaluate(term,a,b):
        return a * b

functionDefs = [Add, Mult]

def createFunctions(codeUnit):
    for functionDef in functionDefs:
        function_builder.createFunction(codeUnit, functionDef)

