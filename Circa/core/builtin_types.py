
from Circa.common import (debug, function_builder)
import builtins, ca_type, ca_function, ca_subroutine

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

def emptyFunction(term):
    pass

def createBuiltinTypes(kernel):
    intType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(intType, 'int')
    ca_type.setInitialize(intType, intInitialize)
    ca_type.setToShortString(intType, intToString)
    kernel.bindName(intType, 'int')

    stringType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(stringType, 'string')
    ca_type.setInitialize(stringType, stringInitialize)
    ca_type.setToShortString(stringType, stringToString)
    kernel.bindName(stringType, 'string')

    boolType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(boolType, 'bool')
    ca_type.setInitialize(boolType, boolInitialize)
    ca_type.setToShortString(boolType, boolToString)
    kernel.bindName(boolType, 'bool')

    floatType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(floatType, 'float')
    ca_type.setInitialize(floatType, floatInitialize)
    ca_type.setToShortString(floatType, floatToString)
    kernel.bindName(floatType, 'float')

    referenceType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(referenceType, 'ref')
    ca_type.setInitialize(referenceType, emptyFunction)
    kernel.bindName(referenceType, 'ref')

    voidType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(voidType, 'void')
    ca_type.setInitialize(voidType, voidInitialize)
    ca_type.setToShortString(voidType, voidToString)
    kernel.bindName(voidType, 'void')

    """
    subroutineType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(subroutineType, 'Subroutine')
    ca_type.setInitialize(subroutineType, ca_subroutine.initializeTerm)
    ca_type.setToShortString(subroutineType, ca_subroutine.toString)
    kernel.bindName(subroutineType, 'Subroutine')
    """
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
    builtins.SUBROUTINE_TYPE = subroutineType
