
#include "common_headers.h"

#include "parser/token.h"
#include "parser/token_stream.h"
#include "bootstrapping.h"
#include "branch.h"
#include "builtin_functions.h"
#include "errors.h"
#include "function.h"
#include "operations.h"
#include "structs.h"
#include "subroutine.h"
#include "term.h"
#include "type.h"

namespace circa {

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
Term* BUILTIN_REFERENCE_TYPE = NULL;
Term* BUILTIN_LIST_TYPE = NULL;

void empty_execute_function(Term*) { }
void empty_alloc_function(Term*) { }

void const_generator(Term* caller)
{
    Function *output = as_function(caller);
    Type* type = as_type(caller->inputs[0]);
    output->name = "const-" + type->name;
    output->outputType = caller->inputs[0];
    output->execute = empty_execute_function;
}

Term* get_global(string name)
{
    if (KERNEL->containsName(name))
        return KERNEL->getNamed(name);

    throw errors::KeyError(name);
}


void bootstrap_kernel()
{
    KERNEL = new Branch();

    // Create const-generator function
    Term* constGenerator = new Term();
    Function_alloc(constGenerator);
    as_function(constGenerator)->name = "const-generator";
    as_function(constGenerator)->pureFunction = true;
    as_function(constGenerator)->execute = const_generator;
    KERNEL->bindName(constGenerator, "const-generator");

    // Create const-Type function
    Term* constTypeFunc = new Term();
    constTypeFunc->function = constGenerator;
    Function_alloc(constTypeFunc);
    as_function(constTypeFunc)->name = "const-Type";
    as_function(constTypeFunc)->pureFunction = true;

    // Create Type type
    Term* typeType = new Term();
    BUILTIN_TYPE_TYPE = typeType;
    typeType->function = constTypeFunc;
    typeType->type = typeType;
    Type_alloc(typeType);
    as_type(typeType)->name = "Type";
    as_type(typeType)->alloc = Type_alloc;
    KERNEL->bindName(typeType, "Type");

    // Implant the Type type
    set_input(constTypeFunc, 0, typeType);
    as_function(constGenerator)->inputTypes.setAt(0, typeType);
    as_function(constTypeFunc)->outputType = typeType;

    // Create const-Function function
    Term* constFuncFunc = new Term();
    constFuncFunc->function = constGenerator;
    Function_alloc(constFuncFunc);
    as_function(constFuncFunc)->name = "const-Function";
    as_function(constFuncFunc)->pureFunction = true;
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Implant const-Function
    constGenerator->function = constFuncFunc;

    // Create Function type
    Term* functionType = new Term();
    BUILTIN_FUNCTION_TYPE = functionType;
    functionType->function = constTypeFunc;
    functionType->type = typeType;
    Type_alloc(functionType);
    as_type(functionType)->name = "Function";
    as_type(functionType)->alloc = Function_alloc;
    KERNEL->bindName(functionType, "Function");

    // Implant Function type
    set_input(constGenerator, 0, typeType);
    set_input(constFuncFunc, 0, functionType);
    constGenerator->type = functionType;
    constFuncFunc->type = functionType;
    constTypeFunc->type = functionType;
    as_function(constGenerator)->outputType = functionType;
    as_function(constFuncFunc)->outputType = functionType;

    // Don't let these terms get updated
    constGenerator->needsUpdate = false;
    constFuncFunc->needsUpdate = false;
    constTypeFunc->needsUpdate = false;
    functionType->needsUpdate = false;
    typeType->needsUpdate = false;
}

void int_alloc(Term* caller)
{
    caller->value = new int;
}
void int_copy(Term* source, Term* dest)
{
    as_int(dest) = as_int(source);
}

void float_alloc(Term* caller)
{
    caller->value = new float;
}
void float_copy(Term* source, Term* dest)
{
    as_float(dest) = as_float(source);
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
void string_copy(Term* source, Term* dest)
{
    as_string(dest) = as_string(source);
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
    BUILTIN_STRING_TYPE = quick_create_type(KERNEL, "string",
            string_alloc, string_tostring, string_copy);
    BUILTIN_INT_TYPE = quick_create_type(KERNEL, "int",
            int_alloc, int_tostring, int_copy);
    BUILTIN_FLOAT_TYPE = quick_create_type(KERNEL, "float",
            float_alloc, float_tostring, float_copy);
    BUILTIN_BOOL_TYPE = quick_create_type(KERNEL, "bool", bool_alloc, bool_tostring);
    BUILTIN_ANY_TYPE = quick_create_type(KERNEL, "any", empty_alloc_function, NULL);
    BUILTIN_VOID_TYPE = quick_create_type(KERNEL, "void", empty_alloc_function, NULL);
    BUILTIN_REFERENCE_TYPE = quick_create_type(KERNEL, "Reference", empty_alloc_function, NULL);
}

void initialize()
{
    try {
        bootstrap_kernel();
        create_builtin_types();

        // Do initialize_term first
        initialize_term(KERNEL);

        initialize_branch(KERNEL);
        initialize_builtin_functions(KERNEL);
        initialize_functions(KERNEL);
        initialize_structs(KERNEL);
        initialize_subroutine(KERNEL);

        initialize_bootstrapped_code(KERNEL);

    } catch (errors::CircaError& e)
    {
        std::cout << "An error occured while initializing." << std::endl;
        std::cout << e.message() << std::endl;
        exit(1);
    }
}

} // namespace circa
