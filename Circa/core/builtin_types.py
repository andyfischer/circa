
from Circa.common import (debug, function_builder)
import builtins, ca_type, ca_function, ca_subroutine

def int_toString(term):
    return str(term.cachedValue)
def int_toSource(term):
    return str(term.cachedValue)

def string_toString(term):
    return term.cachedValue
def string_toSource(term):
    if term.termSyntaxHints.multilineString:
        return '"""' + term.cachedValue + '"""'
    else:
        return "'" + term.cachedValue + "'"

def bool_toString(term):
    return str(term.cachedValue)
def bool_toSource(term):
    return str(term.cachedValue)

def float_toString(term):
    return str(term.cachedValue)
def float_toSource(term):
    return str(term.cachedValue)

def anyToString(term):
    return str(term.cachedValue)

def createBuiltinTypes(kernel):
    intType = ca_type.CircaType()
    intType.name = 'int'
    intType.allocateData = lambda: 0
    intType.toShortString = int_toString
    intType.toSource = int_toSource
    intTerm = kernel.createConstant(builtins.TYPE_TYPE, value=intType)
    kernel.bindName(intTerm, 'int')

    stringType = ca_type.CircaType()
    stringType.name = 'string'
    stringType.allocateData = lambda: ""
    stringType.toShortString = string_toString
    stringType.toSource = string_toSource
    stringTerm = kernel.createConstant(builtins.TYPE_TYPE, value=stringType)
    kernel.bindName(stringTerm, 'string')

    boolType = ca_type.CircaType()
    boolType.name = 'bool'
    boolType.allocateData = lambda: False
    boolType.toShortString = bool_toString
    boolType.toSource = bool_toSource
    boolTerm = kernel.createConstant(builtins.TYPE_TYPE, value=boolType)
    kernel.bindName(boolTerm, 'bool')

    floatType = ca_type.CircaType()
    floatType.name = 'float'
    floatType.allocateData = lambda: 0.0
    floatType.toShortString = float_toString
    floatType.toSource = float_toSource
    floatTerm = kernel.createConstant(builtins.TYPE_TYPE, value=floatType)
    kernel.bindName(floatTerm, 'float')

    referenceType = ca_type.CircaType()
    referenceType.name = 'ref'
    referenceType.allocateData = lambda: None
    referenceTerm = kernel.createConstant(builtins.TYPE_TYPE, value=referenceType)
    kernel.bindName(referenceTerm, 'ref')

    voidType = ca_type.CircaType()
    voidType.name = 'void'
    voidType.allocateData = lambda: None
    voidType.toShortString = lambda: '<void>'
    voidTerm = kernel.createConstant(builtins.TYPE_TYPE, value=voidType)
    kernel.bindName(voidTerm, 'void')

    anyType = ca_type.CircaType()
    anyType.name = 'any'
    anyType.allocateData = lambda: None
    anyTerm = kernel.createConstant(builtins.TYPE_TYPE, value=anyType)
    kernel.bindName(anyTerm, 'any')

    subroutineType = function_builder.importPythonType(kernel, ca_subroutine.CircaSubroutine)

    # Make constant-generator terms for all new functions
    for type in (intTerm, stringTerm, boolTerm, floatTerm, subroutineType):
        kernel.createTerm(builtins.CONST_GENERATOR, [type])

    # Export objects
    builtins.INT_TYPE = intTerm
    builtins.STRING_TYPE = stringTerm
    builtins.BOOL_TYPE = boolTerm
    builtins.FLOAT_TYPE = floatTerm
    builtins.REFERENCE_TYPE = referenceTerm
    builtins.REF_TYPE = referenceTerm # Alias
    builtins.VOID_TYPE = voidTerm
    builtins.ANY_TYPE = anyTerm
    builtins.SUBROUTINE_TYPE = subroutineType
