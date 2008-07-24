
#include "common_headers.h"

#include "branch.h"
#include "builtin_functions.h"
#include "errors.h"
#include "function.h"
#include "globals.h"
#include "operations.h"
#include "structs.h"
#include "subroutine.h"
#include "term.h"
#include "type.h"

Branch* KERNEL = NULL;
Term* BUILTIN_INT_TYPE = NULL;
Term* BUILTIN_FLOAT_TYPE = NULL;
Term* BUILTIN_BOOL_TYPE = NULL;
Term* BUILTIN_STRING_TYPE = NULL;
Term* BUILTIN_TYPE_TYPE = NULL;
Term* BUILTIN_FUNCTION_TYPE = NULL;
Term* BUILTIN_CODEUNIT_TYPE = NULL;
Term* BUILTIN_SUBROUTINE_TYPE = NULL;
Term* BUILTIN_STRUCT_DEFINITION_TYPE = NULL;
Term* BUILTIN_BRANCH_TYPE = NULL;
Term* BUILTIN_ANY_TYPE = NULL;
Term* BUILTIN_VOID_TYPE = NULL;

void const_generator(Term* caller)
{
    Function *output = as_function(caller);
    Type* type = as_type(caller->inputs[0]);
    output->name = "const-" + type->name;
    output->outputType = caller->inputs[0];
    output->execute = empty_execute_function;
}

void empty_alloc_function(Term*,Term*) { }

void bootstrap_kernel()
{
    KERNEL = new Branch();

    // Create const-generator function
    Term* constGenerator = new Term();
    Function_alloc(NULL, constGenerator);
    Function_setName(constGenerator, "const-generator");
    Function_setPureFunction(constGenerator, true);
    Function_setExecute(constGenerator, const_generator);
    KERNEL->bindName(constGenerator, "const-generator");

    // Create const-Type function
    Term* constTypeFunc = new Term();
    constTypeFunc->function = constGenerator;
    Function_alloc(NULL, constTypeFunc);
    Function_setName(constTypeFunc, "const-Type");
    Function_setPureFunction(constTypeFunc, true);

    // Create Type type
    Term* typeType = new Term;
    BUILTIN_TYPE_TYPE = typeType;
    typeType->function = constTypeFunc;
    typeType->type = typeType;
    Type_alloc(typeType, typeType);
    Type_setName     (typeType, "Type");
    Type_setAllocFunc(typeType, Type_alloc);
    KERNEL->bindName(typeType, "Type");

    // Implant the Type type
    set_input(constTypeFunc, 0, typeType);
    Function_setInputType(constGenerator, 0, typeType);
    Function_setOutputType(constTypeFunc, typeType);

    // Create const-Function function
    Term* constFuncFunc = new Term;
    constFuncFunc->function = constGenerator;
    Function_alloc(NULL, constFuncFunc);
    Function_setName        (constFuncFunc, "const-Function");
    Function_setPureFunction(constFuncFunc, true);
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Implant const-Function
    constGenerator->function = constFuncFunc;

    // Create Function type
    Term* functionType = new Term;
    BUILTIN_FUNCTION_TYPE = functionType;
    functionType->function = constTypeFunc;
    functionType->type = typeType;
    Type_alloc(typeType, functionType);
    Type_setName     (functionType, "Function");
    Type_setAllocFunc(functionType, Function_alloc);
    KERNEL->bindName(functionType, "Function");

    // Implant Function type
    set_input(constGenerator, 0, typeType);
    set_input(constFuncFunc, 0, functionType);
    constGenerator->type = functionType;
    constFuncFunc->type = functionType;
    constTypeFunc->type = functionType;
    Function_setOutputType(constGenerator, functionType);
    Function_setOutputType(constFuncFunc, functionType);
}

void int_alloc(Term* type, Term* caller)
{
    caller->value = new int;
}
void int_copy(Term* source, Term* dest)
{
    as_int(dest) = as_int(source);
}

void float_alloc(Term* type, Term* caller)
{
    caller->value = new float;
}
void float_copy(Term* source, Term* dest)
{
    as_float(dest) = as_float(source);
}

void string_alloc(Term* type, Term* caller)
{
    caller->value = new string;
}

void bool_alloc(Term* type, Term* caller)
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

void create_builtin_types()
{
    BUILTIN_STRING_TYPE = quick_create_type(KERNEL, "string", string_alloc, string_tostring);
    BUILTIN_INT_TYPE = quick_create_type(KERNEL, "int",
            int_alloc, int_tostring, int_copy);
    BUILTIN_FLOAT_TYPE = quick_create_type(KERNEL, "float",
            float_alloc, float_tostring, float_copy);
    BUILTIN_BOOL_TYPE = quick_create_type(KERNEL, "bool", bool_alloc, bool_tostring);
    BUILTIN_SUBROUTINE_TYPE = quick_create_type(KERNEL, "Subroutine", Subroutine_alloc, NULL);
    BUILTIN_ANY_TYPE = quick_create_type(KERNEL, "any", empty_alloc_function, NULL);
    BUILTIN_VOID_TYPE = quick_create_type(KERNEL, "void", empty_alloc_function, NULL);
}

void initialize()
{
    try {
        bootstrap_kernel();
        create_builtin_types();
        initialize_builtin_functions(KERNEL);
        initialize_structs(KERNEL);
        initialize_branch(KERNEL);
    } catch (errors::CircaError& e)
    {
        std::cout << "An error occured while initializing." << std::endl;
        std::cout << e.message() << std::endl;
        exit(1);
    }
}
