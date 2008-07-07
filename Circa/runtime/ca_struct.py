
import pdb
import function_builder
from Circa.core import (builtins, ca_type)

class CircaStruct(ca_type.CircaType):
    def __init__(self):
        ca_type.CircaType.__init__(self)
        self.members = []
        self.allocateData = CircaStruct_allocateData
        self.getField = CircaStruct_getField

    def appendMember(self, name, type):
        self.members.append((name,type))


def CircaStruct_allocateData(type):
    result = StructInstance()
    for (memberName, memberType) in type.members:
        memberTypeObj = memberType.value()
        memberAllocateData = ca_type.allocateData(memberType)
        setattr(result, memberName, memberAllocateData(memberTypeObj))
    return result
        
def CircaStruct_getField(term, fieldName):
    return getattr(term.value(), fieldName)

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
        typeObj = term.getType().value()
        if typeObj.getField is None:
            print "Error in get-field, type does not implement getField"
            return
        cxt.setResult(typeObj.getField(term, cxt.input(1)))
   
def createTerms(codeUnit):
    builtins.GET_FIELD = function_builder.importPythonFunction(codeUnit, GetField)
