

import pdb

from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import (debug, function_builder)
from Circa.utils.string_buffer import StringBuffer

class Syntax(object):
    name = "_syntax"
    inputs = ['void']
    output = 'void'

    @staticmethod
    def evaluate(cxt):
        pass

class ToSource(object):
    name = 'to-source'
    inputs = ['ref']
    output = 'string'
    meta = True
    pure = True

    @staticmethod
    def evaluate(cxt):
        buffer = StringBuffer()
        cxt.caller().ast.renderSource(buffer)
        cxt.setResult(str(buffer))

def createFunctions(codeUnit):
    builtins.SYNTAX_FUNC = function_builder.importPythonFunction(codeUnit, Syntax)
    function_builder.importPythonFunction(codeUnit,ToSource)
