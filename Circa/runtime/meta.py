

import pdb

from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import (debug, function_builder)

class Syntax(function_builder.BaseFunction):
    name = "_syntax"
    inputs = ['void']
    output = 'void'

    @staticmethod
    def evaluate(cxt):
        pass

def createFunctions(codeUnit):
    builtins.SYNTAX_FUNC = function_builder.createFunction(codeUnit, Syntax)
