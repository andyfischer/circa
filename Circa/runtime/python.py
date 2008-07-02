
import pdb
from Circa.common import (debug, function_builder)
from Circa.core import builtins

class Import(object):
    name = 'python-import'
    inputs = ['string']
    output = 'void'
    pure = False

    @staticmethod
    def evaluate(filename):
        execfile(filename)

class ImportFunctions(object):
    name = 'python-import-functions'
    inputs = ['string']
    output = 'void'
    pure = False
    meta = True

    @staticmethod
    def evaluate(cxt):
        locals = {}

        execfile(cxt.input(0), globals(), locals)

        # Loop through every symbol defined in this file, and find
        # function definitions.
        for (name,object) in locals.items():
            if isinstance(object, type) and issubclass(object, function_builder.BaseFunction):
                print "Importing " + name
                function_builder.createFunction(cxt.codeUnit(), object)

class RawPythonFunction(object):
    name = 'RawPythonFunction'

    def __init__(self):
        self.func = None

    def call(self, *inputs):
        self.func(*inputs)

class CompileRawFunction(object):
    name = 'compile-raw-python-function'
    inputs = ['string', 'string']
    inputNames = ['pythonSource', 'funcName']
    output = 'RawPythonFunction'
    meta = True

    @staticmethod
    def evaluate(cxt):
        source = cxt.input(0)
        code = compile(source, '<compile-raw-python-function>', 'exec')
        exec code
        cxt.result().func = locals()[cxt.input(1)]

class WrapRawPythonFunction(object):
    name = 'wrap-raw-python-function'
    inputs = ['RawPythonFunction', 'List', 'type', 'bool']
    inputNames = ['functionBody', 'inputTypes', 'outputType', 'pure']
    output = 'Function'
    meta = True

    @staticmethod
    def evaluate(cxt):
        pythonFunc = cxt.input(0).func
        f = cxt.result()
        f.name = pythonFunc.__name__
        # This method of accessing the list of input types is very hacky.
        # Can't do this the regular way b/c list contents are passed as values.
        # Need some more sophisticated solution.
        f.inputTypes = list(cxt.inputTerm(1).inputs)
        f.outputType = cxt.inputTerm(2)
        f.pureFunction = cxt.input(3)

        def evaluateFunc(cxt):
            cxt.setResult(pythonFunc(*cxt.inputs()))
        f.evaluateFunc = evaluateFunc

def createFunctions(codeUnit):
    function_builder.importPythonFunction(codeUnit, Import)
    function_builder.importPythonFunction(codeUnit, ImportFunctions)
    function_builder.importPythonType(codeUnit, RawPythonFunction)
    function_builder.importPythonFunction(codeUnit, CompileRawFunction)
    function_builder.importPythonFunction(codeUnit, WrapRawPythonFunction)
