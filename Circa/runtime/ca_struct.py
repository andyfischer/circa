
import pdb
import function_builder
from Circa.core import (builtins, ca_type)

class CircaStruct(ca_type.CircaType):
    def __init__(self):
        ca_type.CircaType.__init__(self)
        self.members = []
        self.allocateData = CircaStruct_allocateData

    def appendMember(self, name, type):
        self.members.append((name,type))


def CircaStruct_allocateData(type):
    result = StructInstance()
    for (memberName, memberType) in type.members:
        memberTypeObj = memberType.value()
        memberAllocateData = ca_type.allocateData(memberType)
        setattr(result, memberName, memberAllocateData(memberTypeObj))
    return result
        
def CircaStruct_setField(term, name, value):
    setattr(term.value(), name, value)

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
    inputNames = ['term', 'name']
    output = 'any'
    meta = True

    @staticmethod
    def evaluate(cxt):
        term = cxt.inputTerm(0)
        cxt.setResult(getattr(term.value(), cxt.input(1)))

class SetField(object):
    name = 'set-field'
    inputs = ['ref', 'string', 'any']
    inputNames = ['term', 'name', 'value']
    output = 'void'
    meta = True
    pure = False

    @staticmethod
    def evaluate(cxt):
        term = cxt.inputTerm(0)
        setattr(term.value(), cxt.input(1), cxt.input(2))
        cxt.setResult(None)
   
def createTerms(codeUnit):
    builtins.GET_FIELD = function_builder.importPythonFunction(codeUnit, GetField)
    function_builder.importPythonFunction(codeUnit, SetField)
