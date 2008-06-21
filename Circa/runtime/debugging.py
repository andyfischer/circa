
import pdb

from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import (debug, function_builder)

class Assert(function_builder.BaseFunction):
    name = "assert"
    inputs = ['bool']
    output = 'void'

    @staticmethod
    def evaluate(cxt):
        if not cxt.input(0):
            print "Assert failed on term " + cxt.caller().getIdentifier()

class Print(object):
    name = "print"
    inputs = ['string']
    output = 'void'
    pure = False

    @staticmethod
    def evaluate(s):
        print s

def createFunctions(codeUnit):
    function_builder.createFunction(codeUnit, Assert)
    function_builder.importPythonFunction(codeUnit, Print)
