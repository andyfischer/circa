#
# Defines Circa functions for numeric comparison and equality checking

from Circa.common import function_builder
from Circa.core import builtins

class Equals(object):
    name = 'equals'
    inputs = ['int', 'int']
    output = 'bool'

    @staticmethod
    def evaluate(a,b):
        return a == b

class NotEquals(object):
    name = 'not_equals'
    inputs = ['int', 'int']
    output = 'bool'

    @staticmethod
    def evaluate(a,b):
        return a != b

class LessThan(object):
    name = 'less_than'
    inputs = ['float', 'float']
    output = 'bool'

    @staticmethod
    def evaluate(a,b):
        return a < b

class LessThanOrEquals(object):
    name = 'less_than_eq'
    inputs = ['float', 'float']
    output = 'bool'

    @staticmethod
    def evaluate(a,b):
        return a <= b

class GreaterThan(object):
    name = 'greater_than'
    inputs = ['float', 'float']
    output = 'bool'

    @staticmethod
    def evaluate(a,b):
        return a > b
    
class GreaterThanOrEquals(object):
    name = 'greater_than_eq'
    inputs = ['float', 'float']
    output = 'bool'

    @staticmethod
    def evaluate(a,b):
        return a >= b
    

def createFunctions(codeUnit):
    for functionDef in (Equals, NotEquals, LessThan, 
            LessThanOrEquals, GreaterThan, GreaterThanOrEquals):
        function_builder.importPythonFunction(codeUnit, functionDef)
