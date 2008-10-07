// Copyright 2008 Andrew Fischer

#include <iostream>
#include <fstream>

#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "list.h"
#include "operations.h"
#include "tokenizer.h"
#include "token_stream.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {

Branch* KERNEL = NULL;
Term* CONST_GENERATOR = NULL;
Term* INT_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* CODEUNIT_TYPE = NULL;
Term* BRANCH_TYPE = NULL;
Term* ANY_TYPE = NULL;
Term* VOID_TYPE = NULL;
Term* REFERENCE_TYPE = NULL;
Term* LIST_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* CONSTANT_INT = NULL;
Term* CONSTANT_FLOAT = NULL;
Term* CONSTANT_STRING = NULL;
Term* CONSTANT_BOOL = NULL;
Term* CONSTANT_0 = NULL;
Term* CONSTANT_1 = NULL;
Term* CONSTANT_2 = NULL;
Term* CONSTANT_TRUE = NULL;
Term* CONSTANT_FALSE = NULL;
Term* UNKNOWN_FUNCTION = NULL;

void empty_evaluate_function(Term*) { }
void empty_alloc_function(Term*) { }

void const_generator(Term* caller)
{
    assert(caller->inputs[0] != NULL);

    Function& output = as_function(caller);
    Type* type = as_type(caller->inputs[0]);
    output.name = "const-" + type->name;
    output.outputType = caller->inputs[0];
    output.evaluate = empty_evaluate_function;
}

Term* get_global(std::string name)
{
    if (KERNEL->containsName(name))
        return KERNEL->getNamed(name);

    return NULL;
}

void bootstrap_kernel()
{
    KERNEL = new Branch();

    // Create const-generator function
    CONST_GENERATOR = new Term();
    Function::alloc(CONST_GENERATOR);
    as_function(CONST_GENERATOR).name = "const-generator";
    as_function(CONST_GENERATOR).pureFunction = true;
    as_function(CONST_GENERATOR).evaluate = const_generator;
    KERNEL->bindName(CONST_GENERATOR, "const-generator");

    // Create const-Type function
    Term* constTypeFunc = new Term();
    constTypeFunc->function = CONST_GENERATOR;
    Function::alloc(constTypeFunc);
    as_function(constTypeFunc).name = "const-Type";
    as_function(constTypeFunc).pureFunction = true;

    // Create Type type
    Term* typeType = new Term();
    TYPE_TYPE = typeType;
    typeType->function = constTypeFunc;
    typeType->type = typeType;
    initialize_type_type(typeType);
    KERNEL->bindName(typeType, "Type");

    // Implant the Type type
    set_input(constTypeFunc, 0, typeType);
    as_function(CONST_GENERATOR).inputTypes.setAt(0, typeType);
    as_function(constTypeFunc).outputType = typeType;

    // Create const-Function function
    Term* constFuncFunc = new Term();
    constFuncFunc->function = CONST_GENERATOR;
    Function::alloc(constFuncFunc);
    as_function(constFuncFunc).name = "const-Function";
    as_function(constFuncFunc).pureFunction = true;
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Implant const-Function
    CONST_GENERATOR->function = constFuncFunc;

    // Create Function type
    Term* functionType = new Term();
    FUNCTION_TYPE = functionType;
    functionType->function = constTypeFunc;
    functionType->type = typeType;
    as_type(typeType)->alloc(functionType);
    as_type(functionType)->name = "Function";
    as_type(functionType)->alloc = Function::alloc;
    as_type(functionType)->duplicate = Function::duplicate;
    as_type(functionType)->dealloc = Function::dealloc;
    KERNEL->bindName(functionType, "Function");

    // Implant Function type
    set_input(CONST_GENERATOR, 0, typeType);
    set_input(constFuncFunc, 0, functionType);
    CONST_GENERATOR->type = functionType;
    constFuncFunc->type = functionType;
    constTypeFunc->type = functionType;
    as_function(CONST_GENERATOR).outputType = functionType;
    as_function(constFuncFunc).outputType = functionType;

    // Don't let these terms get updated
    CONST_GENERATOR->needsUpdate = false;
    constFuncFunc->needsUpdate = false;
    constTypeFunc->needsUpdate = false;
    functionType->needsUpdate = false;
    typeType->needsUpdate = false;
}

void print__evaluate(Term* caller)
{
    std::cout << as_string(caller->inputs[0]) << std::endl;
}

void add__evaluate(Term* caller)
{
    as_float(caller) = as_float(caller->inputs[0]) + as_float(caller->inputs[1]);
}

void mult__evaluate(Term* caller)
{
    as_float(caller) = as_float(caller->inputs[0]) * as_float(caller->inputs[1]);
}

void string_concat__evaluate(Term* caller)
{
    as_string(caller) = as_string(caller->inputs[0]) + as_string(caller->inputs[1]);
}

void if_expr__evaluate(Term* caller)
{
    int index = as_bool(caller->inputs[0]) ? 1 : 2;
    Term *result = caller->inputs[index];
    change_type(caller, result->type);
    recycle_value(caller->inputs[index], caller);
}

void create_list__evaluate(Term* caller)
{
    as_list(caller).clear();

    for (unsigned int i=0; i < caller->inputs.count(); i++) {
        as_list(caller).append(caller->inputs[i]);
    }
}

void range__evaluate(Term* caller)
{
    unsigned int max = as_int(caller->inputs[0]);

    as_list(caller).clear();

    for (unsigned int i=0; i < max; i++) {
        as_list(caller).append(constant_int(*caller->owningBranch, i));
    }
}

void list_append__evaluate(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    as_list(caller).append(caller->inputs[1]);
}

void list_apply__evaluate(Term* caller)
{
    as_function(caller->inputs[0]);
    List& list = as_list(caller->inputs[1]);

    as_list(caller).clear();

    for (int i=0; i < list.count(); i++) {
        Term* result = apply_function(*caller->owningBranch, caller->inputs[0], ReferenceList(list.get(i)));

        result->eval();

        as_list(caller).append(result);
    }
}

void read_text_file__evaluate(Term* caller)
{
    std::string filename = as_string(caller->inputs[0]);
    std::ifstream file;
    file.open(filename.c_str(), std::ios::in);
    std::stringstream contents;
    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) {
        if (!firstLine)
            contents << "\n";
        contents << line;
        firstLine = false;
    }
    file.close();
    as_string(caller) = contents.str();
}

void write_text_file__evaluate(Term* caller)
{
    std::string filename = as_string(caller->inputs[0]);
    std::string contents = as_string(caller->inputs[1]);
    std::ofstream file;
    file.open(filename.c_str(), std::ios::out);
    file << contents;
    file.close();
}

void to_string__evaluate(Term* caller)
{
    as_string(caller) = caller->inputs[0]->toString();
}

void unknown_function__evaluate(Term* caller)
{
    std::cout << "Warning, calling an unknown function: "
        << as_string(caller->state) << std::endl;
}

void initialize_constants()
{
    BRANCH_TYPE = quick_create_cpp_type<Branch>(KERNEL, "Branch");
    LIST_TYPE = quick_create_cpp_type<List>(KERNEL, "List");
    as_type(LIST_TYPE)->toString = List__toString;

    CONSTANT_INT = get_const_function(*KERNEL, INT_TYPE);
    CONSTANT_FLOAT = get_const_function(*KERNEL, FLOAT_TYPE);
    CONSTANT_STRING = get_const_function(*KERNEL, STRING_TYPE);
    CONSTANT_BOOL = get_const_function(*KERNEL, BOOL_TYPE);

    CONSTANT_0 = constant_float(*KERNEL, 0);
    CONSTANT_1 = constant_float(*KERNEL, 1);
    CONSTANT_2 = constant_float(*KERNEL, 2);

    CONSTANT_TRUE = apply_function(*KERNEL, BOOL_TYPE, ReferenceList());
    as_bool(CONSTANT_TRUE) = true;
    KERNEL->bindName(CONSTANT_TRUE, "true");
    CONSTANT_FALSE = apply_function(*KERNEL, BOOL_TYPE, ReferenceList());
    as_bool(CONSTANT_FALSE) = false;
    KERNEL->bindName(CONSTANT_FALSE, "false");
}

void initialize_builtin_functions(Branch* code)
{
    quick_create_function(code, "add", add__evaluate,
            ReferenceList(FLOAT_TYPE, FLOAT_TYPE), FLOAT_TYPE);
    quick_create_function(code, "mult", mult__evaluate,
            ReferenceList(FLOAT_TYPE, FLOAT_TYPE), FLOAT_TYPE);
    quick_create_function(code, "concat", string_concat__evaluate, ReferenceList(STRING_TYPE, STRING_TYPE), STRING_TYPE);
    quick_create_function(code, "print", print__evaluate, ReferenceList(STRING_TYPE), VOID_TYPE);
    quick_create_function(code, "if-expr", if_expr__evaluate,
        ReferenceList(BOOL_TYPE, ANY_TYPE, ANY_TYPE), ANY_TYPE);
    quick_create_function(code, "list", create_list__evaluate, ReferenceList(ANY_TYPE), LIST_TYPE);
    quick_create_function(code, "range", range__evaluate, ReferenceList(INT_TYPE), LIST_TYPE);
    quick_create_function(code, "list-append", list_append__evaluate, ReferenceList(LIST_TYPE, ANY_TYPE), LIST_TYPE);
    quick_create_function(code, "list-apply", list_apply__evaluate, ReferenceList(FUNCTION_TYPE, LIST_TYPE), LIST_TYPE);
    quick_create_function(code, "read-text-file", read_text_file__evaluate,
            ReferenceList(STRING_TYPE), STRING_TYPE);
    quick_create_function(code, "write-text-file", write_text_file__evaluate,
            ReferenceList(STRING_TYPE, STRING_TYPE), VOID_TYPE);
    quick_create_function(code, "to-string", to_string__evaluate,
        ReferenceList(ANY_TYPE), STRING_TYPE);
    UNKNOWN_FUNCTION = quick_create_function(code, "unknown-function",
            unknown_function__evaluate,
            ReferenceList(ANY_TYPE), ANY_TYPE);
    as_function(UNKNOWN_FUNCTION).stateType = STRING_TYPE;
}

void initialize()
{
    try {
        bootstrap_kernel();
        initialize_primitive_types(KERNEL);
        initialize_constants();
        initialize_compound_types(KERNEL);

        // Then everything else:
        initialize_builtin_functions(KERNEL);
        initialize_functions(KERNEL);

    } catch (errors::CircaError& e)
    {
        std::cout << "An error occured while initializing." << std::endl;
        std::cout << e.message() << std::endl;
        exit(1);
    }
}

void shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}

} // namespace circa
