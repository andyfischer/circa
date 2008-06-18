
# Defines various binary functions

from Circa.common import function_builder
from Circa.core import builtins

class And(object):
    name = "and"
    inputs = ['bool', 'bool']
    output = 'bool'

    @staticmethod
    def evaluate(a,b):
        return a and b

class Or(object):
    name = "or"
    inputs = ['bool', 'bool']
    output = 'bool'

    @staticmethod
    def evaluate(a,b):
        return a or b


def createFunctions(codeUnit):
    function_builder.importPythonFunction(codeUnit, And)
    function_builder.importPythonFunction(codeUnit, Or)

    # Create 'true' and 'false' constants
    true = codeUnit.createConstant(builtins.BOOL_TYPE)
    true.cachedValue = True
    codeUnit.bindName(true, 'true')

    false = codeUnit.createConstant(builtins.BOOL_TYPE)
    false.cachedValue = False
    codeUnit.bindName(false, 'false')
