
# Defines various binary functions

import pdb
from Circa.common import function_builder
from Circa.core import builtins

class IfStatement(object):
    name = 'if-statement'
    inputs = ['bool']
    output = 'void'
    pure = False

    def initialize(self):
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
    inputs = ['bool', 'ref', 'ref']
    output = 'ref'
    pure = True

    @staticmethod
    def evaluate(condition, a, b):
        if condition:
            return a
        else:
            return b

def createFunctions(codeUnit):
    builtins.IF_STATEMENT = function_builder.importPythonFunction(codeUnit,
            IfStatement, instanceBased = True)
    builtins.IF_EXPR = function_builder.importPythonFunction(codeUnit, IfExpression)

