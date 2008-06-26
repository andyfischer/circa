"""
Define the CodeUnit class
"""
import pdb

from Circa.core import (ca_codeunit)
from Circa.common import (debug, function_builder)

class Module(object):
    name = 'Module'
    def __init__(self, name="", codeUnit=None):
        debug._assert(isinstance(name, str))

        self.name = name
        self.codeUnit = codeUnit
        self.mutableFile = False

    def toShortString(self):
        return "<Module %s>" % self.name

    def iterateInnerTerms(self):
        return self.codeUnit.iterateInnerTerms()

    def execute(self):
        return self.codeUnit.execute()

    def containsName(self, name):
        return self.codeUnit.containsName(name)

    def getNamed(self, name):
        return self.codeUnit.getNamed(name)

class LoadModule(object):
    name = 'load-module'
    inputs = ['string']
    inputNames = ['filename']
    output = 'Module'
    pure = False

    @staticmethod
    def evaluate(name):
        from Circa import parser
        filename = name + '.ca'

        (errors, codeUnit) = parser.parseFile(filename)

        if errors:
            print len(errors), "parsing errors occured"
            for error in errors:
                print str(error)
            return

        return Module(name, codeUnit)

class ExecModule(object):
    name = 'exec-module'
    inputs = ['Module']
    inputNames = ['module']
    output = 'void'
    pure = False

    @staticmethod
    def evaluate(module):
        module.codeUnit.execute()

def createTerms(codeUnit):
    function_builder.importPythonType(codeUnit, Module)
    function_builder.importPythonFunction(codeUnit, LoadModule)
    function_builder.importPythonFunction(codeUnit, ExecModule)

