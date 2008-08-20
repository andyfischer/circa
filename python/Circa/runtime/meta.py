
import pdb

import Circa
from Circa.core import (builtins, ca_codeunit, ca_function, ca_type, term)
from Circa.common import (branch_syntax, debug, function_builder)
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

        # Check if this is a constant or variable
        if term.isConstant() or term.isVariable():
            type = term.getType()
            result += type.value().toSource(term)
        else:

            # Otherwise, generate source in the standard way
            functionName = term.termSyntaxHints.functionName
            if functionName is None: functionName = '<none>'

            def getInputSource(index):
                if index in term.termSyntaxHints.namedInputs:
                    return term.termSyntaxHints.namedInputs[index]

                inputTerm = term.inputs[index]

                inputToSourceTerm = cxt.codeUnit().createTerm('to-source', [inputTerm])
                inputToSourceTerm.execute()
                return inputToSourceTerm.value()

            inputsAsSource = map(getInputSource, range(len(term.inputs)))
                
            if term.termSyntaxHints.infix:
                result += (inputsAsSource[0]
                        + term.termSyntaxHints.postInputWhitespace[0]
                        + functionName
                        + term.termSyntaxHints.preInputWhitespace[1]
                        + inputsAsSource[1])
            elif term.termSyntaxHints.rightArrow:
                result += (inputsAsSource[0] + ' -> ' + functionName)
            else:
                result += functionName + "("
                for index in range(len(inputsAsSource)):
                    result += (term.termSyntaxHints.preInputWhitespace[index]
                            + inputsAsSource[index]
                            + term.termSyntaxHints.postInputWhitespace[index])
                    if index + 1 < len(inputsAsSource):
                        result += ','
                result += ")"


        cxt.setResult(result)

class CodeToSource(object):
    name = 'code-to-source'
    inputs = ['CodeUnit']
    output = 'string'
    meta = True

    @staticmethod
    def evaluate(cxt):
        codeUnit = cxt.input(0)
        debug._assert(codeUnit is not None)

        buffer = StringBuffer()
        for line in codeUnit.mainBranch.syntax:
            if isinstance(line, term.Term):
                toSourceTerm = cxt.codeUnit().createTerm('to-source', [line])
                toSourceTerm.execute()
                buffer.writeln(toSourceTerm.value())
            elif type(line).__name__ is 'NameBindingLine':
                toSourceTerm = cxt.codeUnit().createTerm('to-source', [line.term])
                toSourceTerm.execute()
                buffer.writeln('%s = %s' % (line.name, toSourceTerm.value()))
            else:
                buffer.writeln(str(line))

        cxt.setResult(str(buffer))

class GetLocal(object):
    name = 'get-local'
    inputs = ['string']
    output = 'ref'
    meta = True

    @staticmethod
    def evaluate(cxt):
        name = cxt.input(0)

        if cxt.codeUnit().containsName(name):
            cxt.setResult(cxt.codeUnit().getNamed(name))
        else:
            cxt.setResult(Circa.getGlobal(name))

class Comment(object):
    name = '_comment'
    inputs = []
    output = 'void'

    @staticmethod
    def evaluate():
        pass

def createFunctions(codeUnit):
    function_builder.importPythonType(codeUnit, ca_codeunit.CodeUnit)
    builtins.HIGH_LEVEL_OPTION_FUNC = (
            function_builder.importPythonFunction(codeUnit, HighLevelOption))
    function_builder.importPythonFunction(codeUnit,ToSource)
    function_builder.importPythonFunction(codeUnit,CodeToSource)
    builtins.COMMENT_FUNC = function_builder.importPythonFunction(codeUnit,Comment)
    function_builder.importPythonFunction(codeUnit,GetLocal)
