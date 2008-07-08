
import pdb

from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import (debug, function_builder)

class Assert(object):
    name = 'assert'
    inputs = ['bool']
    output = 'void'
    meta = True

    @staticmethod
    def evaluate(cxt):
        if not cxt.input(0):
            print "Assert failed on term " + cxt.caller().getIdentifier()

class Print(object):
    name = 'print'
    inputs = ['any']
    output = 'void'
    pure = False

    @staticmethod
    def evaluate(s):
        print s

class PdbTrace(object):
    name = 'pdb-trace'
    inputs = []
    output = 'void'
    pure = False

    @staticmethod
    def evaluate():
        pdb.set_trace()

def createFunctions(codeUnit):
    function_builder.importPythonFunction(codeUnit, Assert)
    function_builder.importPythonFunction(codeUnit, Print)
    function_builder.importPythonFunction(codeUnit, PdbTrace)
