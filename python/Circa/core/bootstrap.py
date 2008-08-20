
import pdb

import builtins, ca_codeunit, ca_function, ca_type, builtin_types

"""
  constant-generator = 
    Type -> ( constant-generator ) -> Function

  constant-Function =
    (empty) -> ( constant-Function ) -> Function


       input            function             result

  FUNCTION_TYPE -> ( CONST_GENERATOR ) -> CONST_FUNC_FUNC
    TYPE_TYPE   -> ( CONST_GENERATOR ) -> CONST_TYPE_FUNC

      (empty)   -> ( CONST_FUNC_FUNC ) -> CONST_GENERATOR

      (empty)   -> ( CONST_TYPE_FUNC ) -> FUNCTION_TYPE
      (empty)   -> ( CONST_TYPE_FUNC ) ->   TYPE_TYPE

"""

def emptyFunction(term):
    pass

def bootstrapKernel(kernel):

    # Create constant-generator function
    constGenerator = kernel._bootstrapEmptyTerm()
    constGenerator.cachedValue = ca_function.CircaFunction_allocateData(None)
    ca_function.setName(constGenerator, "const-generator")
    ca_function.setPureFunction(constGenerator, True)
    kernel.bindName(constGenerator, "_const-generator")

    # Create constant-Type function
    constTypeFunc = kernel._bootstrapEmptyTerm()
    constTypeFunc.functionTerm = constGenerator
    constTypeFunc.cachedValue = ca_function.CircaFunction_allocateData(None)
    kernel.bindName(constTypeFunc, "_const-Type")
    ca_function.setName(constTypeFunc, "const-Type")
    ca_function.setPureFunction(constTypeFunc, True)
    ca_function.setEvaluateFunc(constTypeFunc, emptyFunction)

    # Create Type type
    typeType = kernel._bootstrapEmptyTerm()
    typeType.functionTerm = constTypeFunc
    typeType.cachedValue = ca_type.CircaType_allocateData(typeType)
    ca_type.setName(typeType, "Type")
    ca_type.setAllocateData(typeType, ca_type.CircaType_allocateData)
    # ca_type.setToString(typeType, ca_type.typeToString)
    kernel.bindName(typeType, "type")

    # Implant the Type type
    kernel.setInput(constTypeFunc, 0, typeType)
    ca_function.setInputTypes(constGenerator, [typeType])
    ca_function.setOutputType(constTypeFunc, typeType)

    # Create constant-Function function
    constFuncFunc = kernel._bootstrapEmptyTerm()
    constFuncFunc.functionTerm = constGenerator
    constFuncFunc.cachedValue = ca_function.CircaFunction_allocateData(None)
    ca_function.setName(constFuncFunc, "const-Function")
    ca_function.setPureFunction(constFuncFunc, True)
    ca_function.setEvaluateFunc(constFuncFunc, emptyFunction)
    kernel.bindName(constFuncFunc, "_const-Function")

    # Implant constant-Function
    constGenerator.functionTerm = constFuncFunc

    # Create Function type
    functionType = kernel._bootstrapEmptyTerm()
    functionType.functionTerm = constTypeFunc
    functionType.cachedValue = ca_type.CircaType_allocateData(typeType)
    ca_type.setName(functionType, "Function")
    ca_type.setAllocateData(functionType, ca_function.CircaFunction_allocateData)
    #ca_type.setToStringFunc(functionType, ca_function.toString)
    kernel.bindName(functionType, "Function")

    # Implant Function type
    kernel.setInput(constGenerator, 0, functionType)
    kernel.setInput(constFuncFunc, 0, functionType)
    ca_function.setOutputType(constGenerator, functionType)
    ca_function.setOutputType(constFuncFunc, functionType)

    # Define a function for constant-generator
    def constGeneratorEvaluate(cxt):
        inputType = cxt.inputTerm(0)
        ca_function.setOutputType(cxt.caller(), inputType)
        ca_function.setEvaluateFunc(cxt.caller(), emptyFunction)
        #ca_function.setHasState(cxt.caller(), True)
        ca_function.setName(cxt.caller(), 'const-' + ca_type.name(inputType))

    ca_function.setEvaluateFunc(constGenerator, constGeneratorEvaluate)

    # All done, re-evaluate everything
    kernel._updateAll()
    kernel._recalculateAllUserSets()

    # Export public symbols
    builtins.CONST_GENERATOR = constGenerator
    builtins.CONST_FUNC_FUNC = constFuncFunc
    builtins.CONST_TYPE_FUNC = constTypeFunc
    builtins.FUNCTION_TYPE = functionType
    builtins.TYPE_TYPE = typeType


# Create Kernel code unit
kernel = ca_codeunit.CodeUnit()
builtins.KERNEL = kernel

bootstrapKernel(kernel)

builtin_types.createBuiltinTypes(kernel)

# Create runtime terms
# future: move this to a separate code unit?

from Circa import runtime
runtime.createFunctions(kernel)

builtins.KERNEL_LOADED = True
