
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
    feedbackHandler = 'get-field-feedback'

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

class GetFieldFeedback(object):
    name = 'get-field-feedback'
    inputs = ['ref','ref']
    output = 'void'
    meta = True
    pure = False

    @staticmethod
    def evaluate(cxt):
        get_field_term = cxt.inputTerm(0)
        desired = cxt.input(1)
        target_struct = get_field_term.getInput(0).value()
        field_name = get_field_term.getInput(1).value()
        setattr(target_struct, field_name, desired)
   
def createTerms(codeUnit):
    function_builder.importPythonFunction(codeUnit, GetFieldFeedback)
    builtins.GET_FIELD = function_builder.importPythonFunction(codeUnit, GetField)
    function_builder.importPythonFunction(codeUnit, SetField)
