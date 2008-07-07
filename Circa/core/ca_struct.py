
import function_builder
from Circa.core import (builtins, ca_type)

class CircaStruct(ca_type.CircaType):
    def __init__(self):
        self.members = []
    def appendMember(self, name, type):
        self.members.append((name,type))

    def allocateData(type):
        result = StructInstance()
        for (memberName, memberType) in type.members:
            setattr(result, memberName, type.allocateData())
        return result
        

class StructInstance(object):
    pass

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
