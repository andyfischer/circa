
#include "common_headers.h"

#include "codeunit.h"
#include "function.h"
#include "primitive_types.h"
#include "term.h"
#include "type.h"

CodeUnit* KERNEL;

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

void create_primitive_type(string name, CircaObject* (*allocFunc)(Term*),
    void (*toStringFunc)(ExecContext*))
{
    CircaType* typeObj = CaType_alloc(null)->asType();
    typeObj->name = name;
    typeObj->alloc = allocFunc;

    Term* typeTerm = KERNEL->createConstant(KERNEL->getNamed("Type"), typeObj, NULL);
    KERNEL->bindName(typeTerm, name);

    CircaFunction* toStringObj = CaFunction_alloc(null)->asFunction();
    toStringObj->name = name + "-to-string";
    toStringObj->execute = toStringFunc;
    toStringObj->inputTypes.setAt(0, typeTerm);
    toStringObj->outputTypes.setAt(0, KERNEL->getNamed("string"));

    Term* toStringTerm = KERNEL->createConstant(KERNEL->getNamed("Function"), toStringObj, NULL);
    typeObj->toString = toStringTerm;
}

void create_primitive_types()
{
    create_primitive_type("string", CaString_alloc, CaString_toString);
    create_primitive_type("int", CaInt_alloc, CaInt_toString);

    CircaType* floatType = CaType_alloc(null)->asType();
    floatType->name = "float";
    floatType->alloc = CaFloat_alloc;

    Term* floatTerm = KERNEL->createConstant(KERNEL->getNamed("Type"), floatType, null);
    KERNEL->bindName(floatTerm, "float");

    CircaType* boolType = CaType_alloc(null)->asType();
    boolType->name = "float";
    boolType->alloc = CaFloat_alloc;

    Term* boolTerm = KERNEL->createConstant(KERNEL->getNamed("Type"), boolType, null);
    KERNEL->bindName(boolTerm, "bool");
}

extern "C" {

void initialize()
{
    bootstrap_kernel();
    create_primitive_types();
    std::cout << "Initialized" << std::endl;
}

}
