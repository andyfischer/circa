
from Circa.common import (debug, function_builder)
from Circa.core import builtins
import pdb

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


def createFunctions(codeUnit):
    function_builder.importPythonFunction(codeUnit, Import)
    function_builder.importPythonFunction(codeUnit, ImportFunctions)
