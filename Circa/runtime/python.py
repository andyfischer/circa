
from Circa.common import function_builder
from Circa.core import builtins
import pdb

class Import(function_builder.BaseFunction):
    name = 'python_import'
    inputTypes = [builtins.STRING_TYPE]
    outputType = builtins.VOID_TYPE
    pureFunction = False

    @staticmethod
    def evaluate(cxt):
        execfile(cxt.input(0))

class ImportFunctions(function_builder.BaseFunction):
    name = 'python_import_functions'
    inputTypes = [builtins.STRING_TYPE]
    outputType = builtins.VOID_TYPE
    pureFunction = False

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

def createFunctions(codeUnit):
    function_builder.createFunction(codeUnit, Import)
    function_builder.createFunction(codeUnit, ImportFunctions)
