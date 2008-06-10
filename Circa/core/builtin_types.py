
from Circa.common import debug
import builtins, ca_type, ca_function

def intInitialize(term):
    term.cachedValue = 0
def intToString(term):
    return str(term.cachedValue)
def stringInitialize(term):
    term.cachedValue = ""
def stringToString(term):
    return term.cachedValue
def boolInitialize(term):
    term.cachedValue = False
def boolToString(term):
    return str(term.cachedValue)

def floatInitialize(term):
    term.cachedValue = 0.0
def floatToString(term):
    return str(term.cachedValue)

def voidInitialize(term):
    term.cachedValue = None
def voidToString(term):
    return "<void>"

def createBuiltinTypes(kernel):
    intType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(intType, 'int')
    ca_type.setInitializeFunc(intType, intInitialize)
    ca_type.setToStringFunc(intType, intToString)
    kernel.bindName(intType, 'int')

    stringType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(stringType, 'string')
    ca_type.setInitializeFunc(stringType, stringInitialize)
    ca_type.setToStringFunc(stringType, stringToString)
    kernel.bindName(stringType, 'string')

    boolType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(boolType, 'bool')
    ca_type.setInitializeFunc(boolType, boolInitialize)
    ca_type.setToStringFunc(boolType, boolToString)
    kernel.bindName(boolType, 'bool')

    floatType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(floatType, 'float')
    ca_type.setInitializeFunc(floatType, floatInitialize)
    ca_type.setToStringFunc(floatType, floatToString)
    kernel.bindName(floatType, 'float')

    referenceType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(referenceType, 'ref')
    kernel.bindName(referenceType, 'ref')

    voidType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(voidType, 'void')
    ca_type.setInitializeFunc(voidType, voidInitialize)
    ca_type.setToStringFunc(voidType, voidToString)
    kernel.bindName(voidType, 'void')
    
    # Export objects
    builtins.INT_TYPE = intType
    builtins.STRING_TYPE = stringType
    builtins.BOOL_TYPE = boolType
    builtins.FLOAT_TYPE = floatType
    builtins.REFERENCE_TYPE = referenceType
    builtins.REF_TYPE = builtins.REFERENCE_TYPE
    builtins.VOID_TYPE = voidType
