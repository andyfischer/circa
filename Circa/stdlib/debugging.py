
import pdb

from Circa.builders import function_builder
from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import debug

class Assert(function_builder.BaseFunction):
    name = "assert"
    inputTypes = [builtins.BOOL_TYPE]
    outputType = builtins.VOID_TYPE

    @staticmethod
    def evaluate(cxt):
        if not cxt.input(0):
            print "Assert failed on term " + cxt.caller().getIdentifier()

def createFunctions(codeUnit):
    function_builder.createFunction(codeUnit, Assert)
