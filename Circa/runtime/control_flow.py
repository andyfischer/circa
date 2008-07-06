
# Defines various binary functions

import pdb
from Circa.common import function_builder
from Circa.core import builtins

class IfStatement(object):
    name = 'if-statement'
    inputs = ['bool']
    output = 'void'
    pure = False
    instanceBased = True
    stateType = 'List'

    def __init__(self):
        self.branches = []

    def evaluate(self, condition):
        try:
            if condition:
                for term in self.branches[0]:
                    term.execute()
            else:
                for term in self.branches[1]:
                    term.execute()
        except IndexError:
            pass

class IfExpression(object):
    name = 'if-expr'
    inputs = ['bool', 'any', 'any']
    output = 'any'
    pure = True

    @staticmethod
    def evaluate(condition, a, b):
        if condition:
            return a
        else:
            return b

def createFunctions(codeUnit):
    builtins.IF_STATEMENT = function_builder.importPythonFunction(codeUnit, IfStatement)
    builtins.IF_EXPR = function_builder.importPythonFunction(codeUnit, IfExpression)

