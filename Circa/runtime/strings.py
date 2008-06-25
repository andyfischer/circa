

from Circa.common import (debug, function_builder)

class Concatenate(object):
    name = 'concat'
    inputs = ['string']
    output = 'string'
    pure = False
    variableArgs = True

    @staticmethod
    def evaluate(*inputs):
        return "".join(inputs)

def createFunctions(codeUnit):
    function_builder.importPythonFunction(codeUnit, Concatenate)
