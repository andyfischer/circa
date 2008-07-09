
# Defines various binary functions

import pdb
from Circa.common import function_builder
from Circa.core import (builtins, branch)

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
        pass

class WhileStatement(object):
    name = 'while-statement'
    inputs = ['bool']
    output = 'void'
    pure = False
    instanceBased = True
    stateType = 'List'
    
    def __init__(self):
        self.branch = branch.Branch(None)

    def evaluate(self, condition):
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
    builtins.WHILE_STATEMENT = function_builder.importPythonFunction(codeUnit, WhileStatement)

