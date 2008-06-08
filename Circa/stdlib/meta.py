

import pdb

from Circa.builders import function_builder
from Circa.core import (builtins, ca_function, ca_type)
from Circa import debug

class Whitespace(function_builder.BaseFunction):
    name = "_whitespace"
    inputTypes = [builtins.VOID_TYPE]
    outputType = builtins.VOID_TYPE

    @staticmethod
    def evaluate(cxt):
        pass


functionDefs = [Whitespace]

def createFunctions(codeUnit):
    for functionDef in functionDefs:
        function_builder.createFunction(codeUnit, functionDef)
