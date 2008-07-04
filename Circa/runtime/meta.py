
import pdb

from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import (debug, function_builder)
from Circa.utils.string_buffer import StringBuffer

class HighLevelOption(object):
    name = 'high-level-option'
    inputs = []
    output = 'void'

    @staticmethod
    def evaluate(*inputs):
        pass

class ToSource(object):
    name = 'to-source'
    inputs = ['ref']
    output = 'string'
    meta = True
    pure = True

    @staticmethod
    def evaluate(cxt):
        term = cxt.inputTerm(0)

        # Check if the function has a special handler
        type = term.getType()
        if type.value().toSourceSpecialHandler is not None:
            cxt.setResult(type.toSourceSpecialHandler(term))
            return

        # Otherwise, generate source in the standard way
        functionName = term.termSyntaxHints.functionName
        sourceOfInputs = []
        for inputTerm in term.inputs:
            inputToSourceTerm = cxt.codeUnit().createTerm('to-source', [input])
            sourceOfInputs.append(inputToSourceTerm.value())
            
        cxt.setResult(functionName + "(" + ",".join(sourceOfInputs) + ")")

class ModuleToSource(object):
    name = 'module-to-source'
    inputs = ['Module']
    output = 'string'

    @staticmethod
    def evaluate(module):
        debug._assert(module is not None)
        debug._assert(module.codeUnit is not None)

        buffer = StringBuffer()
        for term in module.codeUnit.mainBranch:
            if term.termSyntaxInfo is not None:
                buffer.writeln(str(term.termSyntaxInfo))
        return str(buffer)


class Comment(object):
    name = '_comment'
    inputs = []
    output = 'void'

    @staticmethod
    def evaluate():
        pass

def createFunctions(codeUnit):
    builtins.HIGH_LEVEL_OPTION_FUNC = (
            function_builder.importPythonFunction(codeUnit, HighLevelOption))
    function_builder.importPythonFunction(codeUnit,ToSource)
    function_builder.importPythonFunction(codeUnit,ModuleToSource)
    builtins.COMMENT_FUNC = function_builder.importPythonFunction(codeUnit,Comment)
