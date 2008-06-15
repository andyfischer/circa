
# Defines Circa functions for numeric comparison and equality checking

from Circa.common import function_builder
from Circa.core import builtins

class Equals(function_builder.BaseFunction):
    name = 'equals'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.BOOL_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        cxt.setResult( cxt.input(0) == cxt.input(1) )

class NotEquals(function_builder.BaseFunction):
    name = 'not_equals'
    inputTypes = [builtins.INT_TYPE, builtins.INT_TYPE]
    outputType = builtins.BOOL_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        cxt.setResult( cxt.input(0) != cxt.input(1) )

class LessThan(function_builder.BaseFunction):
    name = 'less_than'
    inputTypes = [builtins.FLOAT_TYPE, builtins.FLOAT_TYPE]
    outputType = builtins.BOOL_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        cxt.setResult( cxt.input(0) < cxt.input(1) )

class GreaterThan(function_builder.BaseFunction):
    name = 'greater_than'
    inputTypes = [builtins.FLOAT_TYPE, builtins.FLOAT_TYPE]
    outputType = builtins.BOOL_TYPE
    pureFunction = True

    @staticmethod
    def evaluate(cxt):
        cxt.setResult( cxt.input(0) > cxt.input(1) )


functionDefs = [Equals, NotEquals, LessThan, GreaterThan]

def createFunctions(codeUnit):
    for functionDef in functionDefs:
        function_builder.createFunction(codeUnit, functionDef)


