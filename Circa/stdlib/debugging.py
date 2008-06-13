
import pdb

from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import (debug, function_builder)

class Assert(function_builder.BaseFunction):
    name = "assert"
    inputTypes = [builtins.BOOL_TYPE]
    outputType = builtins.VOID_TYPE

    @staticmethod
    def evaluate(cxt):
        if not cxt.input(0):
            print "Assert failed on term " + cxt.caller().getIdentifier()

class Print(function_builder.BaseFunction):
    name = "print"
    inputTypes = [builtins.STRING_TYPE]
    outputType = builtins.VOID_TYPE
    pureFunction = False
    hasState = False

    @staticmethod
    def evaluate(cxt):
        print cxt.input(0)

def createFunctions(codeUnit):
    function_builder.createFunction(codeUnit, Assert)
    function_builder.createFunction(codeUnit, Print)
