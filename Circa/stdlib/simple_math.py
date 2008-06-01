#
# Defines Circa funcitons for arithmetic

from Circa.builders import function_builder
from Circa.core import builtins

class Add(function_builder.BaseFunction):
    name = 'add'

    @staticmethod
    def evaluate(a,b):
        return a + b

class Mult(function_builder.BaseFunction):
    name = 'mult'

    @staticmethod
    def evaluate(a,b):
        return a * b

functionDefs = [Add, Mult]

def createFunctions(codeUnit):
    for functionDef in functionDefs:
        function_builder.createFunction(codeUnit, functionDef)

