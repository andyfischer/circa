
import builtins, ca_codeunit, ca_function, ca_type

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

def bootstrapKernel():
    kernel = ca_codeunit.CodeUnit()

    # Create constant-generator function
    constGenerator = kernel._bootstrapEmptyTerm()
    ca_function.initializeTerm(constGenerator)
    ca_function.setName(constGenerator, "constant-generator")
    constGenerator.functionTerm = constGenerator

    # Create constant-Type function
    constTypeFunc = kernel._bootstrapEmptyTerm()
    constTypeFunc.functionTerm = constGenerator
    ca_function.initializeTerm(constTypeFunc)
    ca_function.setName(constTypeFunc, "constant-Type")

    # Create Type type
    typeType = kernel._bootstrapEmptyTerm()
    typeType.functionTerm = constTypeFunc
    ca_type.initializeTerm(typeType)
    ca_type.setName(typeType, "Type")
    kernel.bindName("Type", typeType)

    # Implant the Type type into constant-Type
    kernel.setInput(constTypeFunc, 0, typeType)
    ca_function.setInputType(constGenerator, 0, typeType)

    # Create constant-Function function
    constFuncFunc = kernel._bootstrapEmptyTerm()
    constFuncFunc.functionTerm = constGenerator
    ca_function.initializeTerm(constFuncFunc)
    ca_function.setName(constFuncFunc, "constant-Function")

    # Implant constant-Function as an input to constant-generator
    kernel.setInput(constGenerator, 0, constFuncFunc)

    # Create Function type
    functionType = kernel._bootstrapEmptyTerm()
    functionType.functionTerm = constFuncFunc
    ca_type.initializeTerm(functionType)
    ca_type.setName(functionType, "Function")
    ca_type.setInitializeFunc(functionType, ca_function.initializeTerm)
    ca_type.setToStringFunc(functionType, ca_function.toString)
    kernel.bindName("Function", functionType)

    # Implant Function type into various places
    kernel.setInput(constFuncFunc, 0, functionType)
    ca_function.setOutputType(constGenerator, functionType)
    ca_function.setOutputType(constFuncFunc, functionType)
    ca_function.setOutputType(constTypeFunc, functionType)

    # Define a function for constant-generator
    def constGeneratorEvaluate(term):
        inputType = term.getInput(0)
        ca_function.setOutputType(term, inputType)
    ca_function.setEvaluateFunc(constGenerator, constGeneratorEvaluate)

    # Export public symbols
    builtins.KERNEL = kernel
    builtins.CONST_GENERATOR = constGenerator
    builtins.CONST_FUNC_FUNC = constFuncFunc
    builtins.CONST_TYPE_FUNC = constTypeFunc
    builtins.FUNCTION_TYPE = functionType
    builtins.TYPE_TYPE = typeType

bootstrapKernel()
