
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

        result = ""

        # Prepend a name binding, if any
        if term.termSyntaxHints.nameBinding is not None:
            result = term.termSyntaxHints.nameBinding + ' = '

        # Check if this is a constant or variable
        if term.isConstant() or term.isVariable():
            type = term.getType()
            result += type.value().toSource(term)
        else:

            # Otherwise, generate source in the standard way
            functionName = term.termSyntaxHints.functionName
            if functionName is None: functionName = '<none>'

            sourceOfInputs = []
            for inputTerm in term.inputs:
                inputToSourceTerm = cxt.codeUnit().createTerm('to-source', [inputTerm])
                sourceOfInputs.append(inputToSourceTerm.value())
                
            if term.termSyntaxHints.infix:
                result += (sourceOfInputs[0] + ' ' + functionName + ' ' + sourceOfInputs[1])
            else:
                result += (functionName + "(" + ",".join(sourceOfInputs) + ")")

        cxt.setResult(result)

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
