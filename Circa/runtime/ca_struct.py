
import function_builder
from Circa.core import builtins

class CircaStructDefinition(object):
    name = 'struct-definition'

    def __init__(self):
        self.members = []

    def appendMember(self, name, type):
        self.members.append((name,type))

class DynamicObjectType(object):
    name = 'dynamic-object'

    def __init__(self):
        self.members = {}

    @staticmethod
    def getField(term, name):
        pass


class GetField(object):
    name = 'get-field'
    inputs = ['ref', 'string']
    inputNames = ['term', 'fieldName']
    output = 'any'
    meta = True

    @staticmethod
    def evaluate(cxt):
        term = cxt.inputTerm(0)
        type = term.getType()
        if type.getField is None:
            print "Error in get-field, type does not implement getField"
            return
        cxt.setResult(type.getField(term, cxt.input(1)))
   
def createTerms(codeUnit):
    builtins.GET_FIELD = function_builder.importPythonFunction(codeUnit, GetField)
