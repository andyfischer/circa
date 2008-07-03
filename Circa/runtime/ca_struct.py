
import function_builder
from Circa.core import builtins

class CircaStructDefinition(object):
    name = 'struct-definition'

    def __init__(self):
        self.members = []

    def appendMember(self, name, type):
        self.members.append((name,type))

class GetField(object):
    name = 'get-field'
    inputs = ['ref', 'string']
    inputNames = ['term', 'fieldName']
    output = 'any'
    meta = True

    @staticmethod
    def evaluate(cxt):
        print "todo"
   
def createTerms(codeUnit):
    builtins.GET_FIELD = function_builder.importPythonFunction(codeUnit, GetField)
