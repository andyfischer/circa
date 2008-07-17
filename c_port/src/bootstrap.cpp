
#include "common_headers.h"

#include "codeunit.h"
#include "errors.h"
#include "function.h"
#include "globals.h"
#include "primitive_types.h"
#include "term.h"
#include "type.h"

CodeUnit* KERNEL = NULL;
Term* BUILTIN_INT_TYPE = NULL;
Term* BUILTIN_FLOAT_TYPE = NULL;
Term* BUILTIN_BOOL_TYPE = NULL;
Term* BUILTIN_STRING_TYPE = NULL;
Term* BUILTIN_TYPE_TYPE = NULL;
Term* BUILTIN_FUNCTION_TYPE = NULL;
Term* BUILTIN_CODEUNIT_TYPE = NULL;


void bootstrap_kernel()
{
    KERNEL = new CodeUnit();

    // Create const-generator function
    Term* constGenerator = KERNEL->_bootstrapEmptyTerm();
    constGenerator->outputValue = CaFunction_alloc(null);
    CaFunction_setName(constGenerator, "const-generator");
    CaFunction_setPureFunction(constGenerator, true);
    KERNEL->bindName(constGenerator, "const-generator");

    // Create const-Type function
    Term* constTypeFunc = KERNEL->_bootstrapEmptyTerm();
    constTypeFunc->function = constGenerator;
    constTypeFunc->outputValue = CaFunction_alloc(null);
    CaFunction_setName(constTypeFunc, "const-Type");
    CaFunction_setPureFunction(constTypeFunc, true);

    // Create Type type
    Term* typeType = KERNEL->_bootstrapEmptyTerm();
    typeType->function = constTypeFunc;
    typeType->outputValue = CaType_alloc(null);
    CaType_setName     (typeType, "Type");
    CaType_setAllocFunc(typeType, CaType_alloc);
    KERNEL->bindName(typeType, "Type");

    // Implant the Type type
    KERNEL->setInput(constTypeFunc, 0, typeType);
    CaFunction_setInputType(constGenerator, 0, typeType);
    CaFunction_setOutputType(constTypeFunc, 0, typeType);

    // Create const-Function function
    Term* constFuncFunc = KERNEL->_bootstrapEmptyTerm();
    constFuncFunc->function = constGenerator;
    constFuncFunc->outputValue = CaFunction_alloc(null);
    CaFunction_setName        (constFuncFunc, "const-Function");
    CaFunction_setPureFunction(constFuncFunc, true);
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Implant const-Function
    constGenerator->function = constFuncFunc;

    // Create Function type
    Term* functionType = KERNEL->_bootstrapEmptyTerm();
    functionType->function = constTypeFunc;
    functionType->outputValue = CaType_alloc(null);
    CaType_setName     (functionType, "Function");
    CaType_setAllocFunc(functionType, CaFunction_alloc);
    KERNEL->bindName(functionType, "Function");

    // Implant Function type
    KERNEL->setInput(constGenerator, 0, typeType);
    KERNEL->setInput(constFuncFunc, 0, functionType);
    CaFunction_setOutputType(constGenerator, 0, functionType);
    CaFunction_setOutputType(constFuncFunc, 0, functionType);
}

Term* create_primitive_type(string name, CircaObject* (*allocFunc)(Term*),
    void (*toStringFunc)(ExecContext*))
{
    CircaType* typeObj = CaType_alloc(null)->asType();
    typeObj->name = name;
    typeObj->alloc = allocFunc;

    Term* typeTerm = KERNEL->createConstant(GetGlobal("Type"), typeObj, NULL);
    KERNEL->bindName(typeTerm, name);

    // Create to-string function
    CircaFunction* toStringObj = CaFunction_alloc(null)->asFunction();
    toStringObj->name = name + "-to-string";
    toStringObj->execute = toStringFunc;
    toStringObj->inputTypes.setAt(0, typeTerm);
    toStringObj->outputTypes.setAt(0, GetGlobal("string"));

    Term* toStringTerm = KERNEL->createConstant(GetGlobal("Function"), toStringObj, NULL);
    typeObj->toString = toStringTerm;
}

void create_primitive_types()
{
    BUILTIN_STRING_TYPE = create_primitive_type("string", CaString_alloc, CaString_toString);
    BUILTIN_INT_TYPE = create_primitive_type("int", CaInt_alloc, CaInt_toString);
    BUILTIN_FLOAT_TYPE = create_primitive_type("float", CaFloat_alloc, CaFloat_toString);
    BUILTIN_BOOL_TYPE = create_primitive_type("bool", CaBool_alloc, CaBool_toString);
}

extern "C" {

void initialize()
{
    try {
        bootstrap_kernel();
        create_primitive_types();
        std::cout << "Initialized" << std::endl;
    } catch (errors::CircaError& e)
    {
        std::cout << e.what() << std::endl;
    }
}

}
