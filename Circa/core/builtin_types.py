
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

def createBuiltinTypes(kernel):
    intType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(intType, 'Int')
    ca_type.setInitializeFunc(intType, intInitialize)
    ca_type.setToStringFunc(intType, intToString)
    kernel.bindName(intType, 'Int')

    stringType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(stringType, 'String')
    ca_type.setInitializeFunc(stringType, stringInitialize)
    ca_type.setToStringFunc(stringType, stringToString)
    kernel.bindName(stringType, 'String')

    boolType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(boolType, 'Bool')
    ca_type.setInitializeFunc(boolType, boolInitialize)
    ca_type.setToStringFunc(boolType, boolToString)
    kernel.bindName(boolType, 'Bool')

    floatType = kernel.createConstant(builtins.TYPE_TYPE)
    ca_type.setName(floatType, 'Float')
    ca_type.setInitializeFunc(floatType, floatInitialize)
    ca_type.setToStringFunc(floatType, floatToString)
    kernel.bindName(floatType, 'Float')
    
    # Export objects
    builtins.INT_TYPE = intType
    builtins.STRING_TYPE = stringType
    builtins.BOOL_TYPE = boolType
    builtins.FLOAT_TYPE = floatType
