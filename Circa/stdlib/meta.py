

import pdb

from Circa.builders import function_builder
from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import debug

class Syntax(function_builder.BaseFunction):
    name = "_syntax"
    inputTypes = [builtins.VOID_TYPE]
    outputType = builtins.VOID_TYPE

    @staticmethod
    def evaluate(cxt):
        pass

def createFunctions(codeUnit):
    builtins.SYNTAX_FUNC = function_builder.createFunction(codeUnit, Syntax)
