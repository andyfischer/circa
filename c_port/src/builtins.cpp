
#include "common_headers.h"

#include "codeunit.h"
#include "errors.h"
#include "function.h"
#include "globals.h"
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

void const_generator(Term* caller)
{
    Function *output = as_function(caller);
    Type* type = as_type(caller->inputs[0]);
    output->name = "const-" + type->name;
    output->outputType = caller->inputs[0];
}

void bootstrap_kernel()
{
    KERNEL = new CodeUnit();

    // Create const-generator function
    Term* constGenerator = KERNEL->_bootstrapEmptyTerm();
    Function_alloc(constGenerator);
    Function_setName(constGenerator, "const-generator");
    Function_setPureFunction(constGenerator, true);
    Function_setExecute(constGenerator, const_generator);
    KERNEL->bindName(constGenerator, "const-generator");

    // Create const-Type function
    Term* constTypeFunc = KERNEL->_bootstrapEmptyTerm();
    constTypeFunc->function = constGenerator;
    Function_alloc(constTypeFunc);
    Function_setName(constTypeFunc, "const-Type");
    Function_setPureFunction(constTypeFunc, true);

    // Create Type type
    Term* typeType = KERNEL->_bootstrapEmptyTerm();
    typeType->function = constTypeFunc;
    Type_alloc(typeType);
    Type_setName     (typeType, "Type");
    Type_setAllocFunc(typeType, Type_alloc);
    KERNEL->bindName(typeType, "Type");

    // Implant the Type type
    KERNEL->setInput(constTypeFunc, 0, typeType);
    Function_setInputType(constGenerator, 0, typeType);
    Function_setOutputType(constTypeFunc, typeType);

    // Create const-Function function
    Term* constFuncFunc = KERNEL->_bootstrapEmptyTerm();
    constFuncFunc->function = constGenerator;
    Function_alloc(constFuncFunc);
    Function_setName        (constFuncFunc, "const-Function");
    Function_setPureFunction(constFuncFunc, true);
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Implant const-Function
    constGenerator->function = constFuncFunc;

    // Create Function type
    Term* functionType = KERNEL->_bootstrapEmptyTerm();
    functionType->function = constTypeFunc;
    Type_alloc(functionType);
    Type_setName     (functionType, "Function");
    Type_setAllocFunc(functionType, Function_alloc);
    KERNEL->bindName(functionType, "Function");

    // Implant Function type
    KERNEL->setInput(constGenerator, 0, typeType);
    KERNEL->setInput(constFuncFunc, 0, functionType);
    Function_setOutputType(constGenerator, functionType);
    Function_setOutputType(constFuncFunc, functionType);
}

void int_alloc(Term* caller)
{
    caller->value = new int;
}

void float_alloc(Term* caller)
{
    caller->value = new float;
}

void string_alloc(Term* caller)
{
    caller->value = new string;
}

void bool_alloc(Term* caller)
{
    caller->value = new bool;
}

void int_tostring(Term* caller)
{
    std::stringstream strm;
    strm << as_int(caller->inputs[0]);
    as_string(caller) = strm.str();
}
void float_tostring(Term* caller)
{
    std::stringstream strm;
    strm << as_float(caller->inputs[0]);
    as_string(caller) = strm.str();
}
void string_tostring(Term* caller)
{
    as_string(caller) = as_string(caller->inputs[0]);
}

void bool_tostring(Term* caller)
{
    if (as_bool(caller))
        as_string(caller) = "true";
    else
        as_string(caller) = "false";
}

Term* create_primitive_type(string name, void (*allocFunc)(Term*),
    void (*toStringFunc)(Term*))
{
    Term* typeTerm = KERNEL->createConstant(GetGlobal("Type"), NULL);
    as_type(typeTerm)->name = name;
    as_type(typeTerm)->alloc = allocFunc;
    KERNEL->bindName(typeTerm, name);

    // Create to-string function
    Term* toString = KERNEL->createConstant(GetGlobal("Function"), NULL);
    as_function(toString)->name = name + "-to-string";
    as_function(toString)->execute = toStringFunc;
    as_function(toString)->inputTypes.setAt(0, typeTerm);

    if (GetGlobal("string") == NULL)
        throw errors::InternalError("string type not defined");

    as_function(toString)->outputType = GetGlobal("string");
        
    as_type(typeTerm)->toString = toString;

    return typeTerm;
}

void create_primitive_types()
{
    BUILTIN_STRING_TYPE = create_primitive_type("string", string_alloc, string_tostring);
    BUILTIN_INT_TYPE = create_primitive_type("int", int_alloc, int_tostring);
    BUILTIN_FLOAT_TYPE = create_primitive_type("float", float_alloc, float_tostring);
    BUILTIN_BOOL_TYPE = create_primitive_type("bool", bool_alloc, bool_tostring);
}

void initialize()
{
    try {
        bootstrap_kernel();
        create_primitive_types();
    } catch (errors::CircaError& e)
    {
        std::cout << "An error occured while initializing." << std::endl;
        std::cout << e.message() << std::endl;
        exit(1);
    }
}
