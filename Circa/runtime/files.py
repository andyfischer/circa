
from Circa.common import function_builder

class ReadTextFile(object):
    name = 'read-text-file'
    inputs = ['string']
    inputNames = ['filename']
    output = 'string'
    outputName = 'fileContents'
    pure = False

    @staticmethod
    def evaluate(filename):
        file = open(filename, 'r')
        contents = file.read()
        file.close()
        return contents

class WriteTextFile(object):
    name = 'write-text-file'
    inputs = ['string', 'string']
    inputNames = ['filename', 'contents']
    output = 'void'
    pure = False

    @staticmethod
    def evaluate(filename, contents):
        file = open(filename, 'w')
        file.write(contents)
        file.close()

def createFunctions(codeUnit):
    function_builder.importPythonFunction(codeUnit, ReadTextFile)
    function_builder.importPythonFunction(codeUnit, WriteTextFile)
