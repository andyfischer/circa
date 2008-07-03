
import function_builder
from Circa.core import builtins

class CircaStructDefinition(object):
    name = 'struct-definition'

    def __init__(self):
        self.members = []

    def appendMember(self, name, type):
        self.members.append((name,type))

class DynamicObject(object):
    name = 'dynamic-object'


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
        cxt.setResult(type.getField(term, cxt.input(1)))
   
def createTerms(codeUnit):
    builtins.GET_FIELD = function_builder.importPythonFunction(codeUnit, GetField)
