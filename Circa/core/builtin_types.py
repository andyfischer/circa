
from Circa.common import (debug, function_builder)
import builtins, ca_type, ca_function, ca_subroutine

def intToString(term):
    return str(term.cachedValue)
def stringToString(term):
    return term.cachedValue
def boolToString(term):
    return str(term.cachedValue)

def floatToString(term):
    return str(term.cachedValue)

def voidToString(term):
    return "<void>"

def anyToString(term):
    return str(term.cachedValue)

def createBuiltinTypes(kernel):
    intType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(intType, 'int')
    ca_type.setAllocateData(intType, lambda: 0)
    ca_type.setToShortString(intType, intToString)
    kernel.bindName(intType, 'int')

    stringType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(stringType, 'string')
    ca_type.setAllocateData(stringType, lambda: "")
    ca_type.setToShortString(stringType, stringToString)
    kernel.bindName(stringType, 'string')

    boolType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(boolType, 'bool')
    ca_type.setAllocateData(boolType, lambda: False)
    ca_type.setToShortString(boolType, boolToString)
    kernel.bindName(boolType, 'bool')

    floatType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(floatType, 'float')
    ca_type.setAllocateData(floatType, lambda: 0.0)
    ca_type.setToShortString(floatType, floatToString)
    kernel.bindName(floatType, 'float')

    referenceType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(referenceType, 'ref')
    ca_type.setAllocateData(referenceType, lambda: None)
    kernel.bindName(referenceType, 'ref')

    voidType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(voidType, 'void')
    ca_type.setAllocateData(voidType, lambda: None)
    ca_type.setToShortString(voidType, voidToString)
    kernel.bindName(voidType, 'void')

    anyType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(anyType, 'any')
    ca_type.setAllocateData(anyType, lambda: None)
    ca_type.setToShortString(anyType, anyToString)
    kernel.bindName(anyType, 'any')

    subroutineType = function_builder.importPythonType(kernel, ca_subroutine.CircaSubroutine)

    # Make constant-generator terms for all new functions
    for type in (intType, stringType, boolType, floatType, subroutineType):
        kernel.createTerm(builtins.CONST_GENERATOR, [type])

    # Export objects
    builtins.INT_TYPE = intType
    builtins.STRING_TYPE = stringType
    builtins.BOOL_TYPE = boolType
    builtins.FLOAT_TYPE = floatType
    builtins.REFERENCE_TYPE = referenceType
    builtins.REF_TYPE = referenceType # Alias
    builtins.VOID_TYPE = voidType
    builtins.ANY_TYPE = anyType
    builtins.SUBROUTINE_TYPE = subroutineType
