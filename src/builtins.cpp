
#include <iostream>
#include <fstream>

#include "common_headers.h"

#include "tokenizer.h"
#include "token_stream.h"
#include "libraries/builtin_libraries.h"
#include "bootstrapping.h"
#include "branch.h"
#include "errors.h"
#include "function.h"
#include "operations.h"
#include "structs.h"
#include "subroutine.h"
#include "term.h"
#include "type.h"

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
Term* SUBROUTINE_TYPE = NULL;
Term* STRUCT_DEFINITION_TYPE = NULL;
Term* BRANCH_TYPE = NULL;
Term* ANY_TYPE = NULL;
Term* VOID_TYPE = NULL;
Term* REFERENCE_TYPE = NULL;
Term* LIST_TYPE = NULL;
Term* CONSTANT_INT = NULL;
Term* CONSTANT_STRING = NULL;
Term* CONSTANT_0 = NULL;
Term* CONSTANT_1 = NULL;
Term* CONSTANT_2 = NULL;
Term* CONSTANT_TRUE = NULL;
Term* CONSTANT_FALSE = NULL;

void empty_execute_function(Term*) { }
void empty_alloc_function(Term*) { }
void empty_function(Term*) { }
void empty_copy_function(Term*,Term*) { }

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
    Term* CONST_GENERATOR = new Term();
    Function_alloc(CONST_GENERATOR);
    as_function(CONST_GENERATOR)->name = "const-generator";
    as_function(CONST_GENERATOR)->pureFunction = true;
    as_function(CONST_GENERATOR)->execute = const_generator;
    KERNEL->bindName(CONST_GENERATOR, "const-generator");

    // Create const-Type function
    Term* constTypeFunc = new Term();
    constTypeFunc->function = CONST_GENERATOR;
    Function_alloc(constTypeFunc);
    as_function(constTypeFunc)->name = "const-Type";
    as_function(constTypeFunc)->pureFunction = true;

    // Create Type type
    Term* typeType = new Term();
    TYPE_TYPE = typeType;
    typeType->function = constTypeFunc;
    typeType->type = typeType;
    Type_alloc(typeType);
    as_type(typeType)->name = "Type";
    as_type(typeType)->alloc = Type_alloc;
    KERNEL->bindName(typeType, "Type");

    // Implant the Type type
    set_input(constTypeFunc, 0, typeType);
    as_function(CONST_GENERATOR)->inputTypes.setAt(0, typeType);
    as_function(constTypeFunc)->outputType = typeType;

    // Create const-Function function
    Term* constFuncFunc = new Term();
    constFuncFunc->function = CONST_GENERATOR;
    Function_alloc(constFuncFunc);
    as_function(constFuncFunc)->name = "const-Function";
    as_function(constFuncFunc)->pureFunction = true;
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Implant const-Function
    CONST_GENERATOR->function = constFuncFunc;

    // Create Function type
    Term* functionType = new Term();
    FUNCTION_TYPE = functionType;
    functionType->function = constTypeFunc;
    functionType->type = typeType;
    Type_alloc(functionType);
    as_type(functionType)->name = "Function";
    as_type(functionType)->alloc = Function_alloc;
    as_type(functionType)->copy = Function_copy;
    as_type(functionType)->dealloc = Function_dealloc;
    KERNEL->bindName(functionType, "Function");

    // Implant Function type
    set_input(CONST_GENERATOR, 0, typeType);
    set_input(constFuncFunc, 0, functionType);
    CONST_GENERATOR->type = functionType;
    constFuncFunc->type = functionType;
    constTypeFunc->type = functionType;
    as_function(CONST_GENERATOR)->outputType = functionType;
    as_function(constFuncFunc)->outputType = functionType;

    // Don't let these terms get updated
    CONST_GENERATOR->needsUpdate = false;
    constFuncFunc->needsUpdate = false;
    constTypeFunc->needsUpdate = false;
    functionType->needsUpdate = false;
    typeType->needsUpdate = false;
}

int& as_int(Term* t)
{
    if (t->type != INT_TYPE)
        throw errors::TypeError(t, INT_TYPE);

    return *((int*) t->value);
}

float& as_float(Term* t)
{
    if (t->type != FLOAT_TYPE)
        throw errors::TypeError(t, FLOAT_TYPE);

    return *((float*) t->value);
}

bool& as_bool(Term* t)
{
    if (t->type != BOOL_TYPE)
        throw errors::TypeError(t, BOOL_TYPE);

    return *((bool*) t->value);
}

string& as_string(Term* t)
{
    if (t->type != STRING_TYPE)
        throw errors::TypeError(t, STRING_TYPE);

    if (t->value == NULL)
        throw errors::InternalError("NULL pointer in as_string");

    return *((string*) t->value);
}

void int_alloc(Term* caller)
{
    caller->value = new int;
}
void int_dealloc(Term* caller)
{
    delete (int*) caller->value;
}
void int_copy(Term* source, Term* dest)
{
    as_int(dest) = as_int(source);
}
std::string int_toString(Term* term)
{
    std::stringstream strm;
    strm << as_int(term);
    return strm.str();
}

void float_alloc(Term* caller)
{
    caller->value = new float;
}
void float_dealloc(Term* caller)
{
    delete (float*) caller->value;
}
void float_copy(Term* source, Term* dest)
{
    as_float(dest) = as_float(source);
}
std::string float_toString(Term* term)
{
    std::stringstream strm;
    strm << as_float(term);
    return strm.str();
}

void string_alloc(Term* caller)
{
    caller->value = new string();
}

void string_dealloc(Term* caller)
{
    delete (string*) caller->value;
    caller->value = NULL;
}
std::string string_toString(Term* term)
{
    return as_string(term);
}
void string_copy(Term* source, Term* dest)
{
    as_string(dest) = as_string(source);
}

void bool_alloc(Term* caller)
{
    caller->value = new bool;
}

void bool_dealloc(Term* caller)
{
    delete (bool*) caller->value;
}

void bool_copy(Term* source, Term* dest)
{
    as_bool(dest) = as_bool(source);
}

std::string bool_toString(Term* term)
{
    if (as_bool(term))
        return "true";
    else
        return "false";
}

void reference_alloc(Term* caller)
{
    caller->value = NULL;
}
void reference_dealloc(Term* caller)
{
    caller->value = NULL;
}
Term*& as_reference(Term* term)
{
    return (Term*&) term->value;
}
void reference_copy(Term* source, Term* dest)
{
    dest->value = source->value;
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
    as_list(caller)->clear();

    for (int i=0; i < caller->inputs.count(); i++) {
        as_list(caller)->append(caller->inputs[i]);
    }
}

void range__evaluate(Term* caller)
{
    int max = as_int(caller->inputs[0]);

    as_list(caller)->clear();

    for (int i=0; i < max; i++) {
        as_list(caller)->append(constant_int(caller->owningBranch, i));
    }
}

void list_apply__evaluate(Term* caller)
{
    as_function(caller->inputs[0]);
    TermList* list = as_list(caller->inputs[1]);

    as_list(caller)->clear();

    for (int i=0; i < list->count(); i++) {
        Term* result = apply_function(caller->owningBranch, caller->inputs[0], TermList(list->get(i)));
        execute(result);

        as_list(caller)->append(result);
    }
}

void this_branch__evaluate(Term* caller)
{
    // TOFIX, this will have problems when memory management is implemented
    *as_list(caller) = caller->owningBranch->terms;
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

void initialize_constants()
{
    STRING_TYPE = quick_create_type(KERNEL, "string",
            string_alloc,
            string_dealloc,
            string_copy,
            string_toString);
    INT_TYPE = quick_create_type(KERNEL, "int",
            int_alloc,
            int_dealloc,
            int_copy,
            int_toString);
    FLOAT_TYPE = quick_create_type(KERNEL, "float",
            float_alloc, float_dealloc, float_copy, float_toString);
    BOOL_TYPE = quick_create_type(KERNEL, "bool",
            bool_alloc, bool_dealloc, bool_copy, bool_toString);
    ANY_TYPE = quick_create_type(KERNEL, "any",
            empty_function, empty_function, NULL);
    VOID_TYPE = quick_create_type(KERNEL, "void",
            empty_function, empty_function, empty_copy_function);
    REFERENCE_TYPE = quick_create_type(KERNEL, "Reference",
            reference_alloc,
            reference_dealloc,
            reference_copy);

    CONSTANT_INT = get_const_function(KERNEL, INT_TYPE);
    CONSTANT_STRING = get_const_function(KERNEL, STRING_TYPE);

    CONSTANT_0 = constant_float(KERNEL, 0);
    CONSTANT_1 = constant_float(KERNEL, 1);
    CONSTANT_2 = constant_float(KERNEL, 2);

    CONSTANT_TRUE = apply_function(KERNEL, BOOL_TYPE, TermList());
    as_bool(CONSTANT_TRUE) = true;
    KERNEL->bindName(CONSTANT_TRUE, "true");
    CONSTANT_FALSE = apply_function(KERNEL, BOOL_TYPE, TermList());
    as_bool(CONSTANT_FALSE) = false;
    KERNEL->bindName(CONSTANT_FALSE, "false");
}

void initialize_builtin_functions(Branch* code)
{
    quick_create_function(code, "add", add__evaluate,
            TermList(FLOAT_TYPE, FLOAT_TYPE), FLOAT_TYPE);
    quick_create_function(code, "mult", mult__evaluate,
            TermList(FLOAT_TYPE, FLOAT_TYPE), FLOAT_TYPE);
    quick_create_function(code, "concat", string_concat__evaluate, TermList(STRING_TYPE, STRING_TYPE), STRING_TYPE);
    quick_create_function(code, "print", print__evaluate, TermList(STRING_TYPE), VOID_TYPE);
    quick_create_function(code, "if-expr", if_expr__evaluate,
        TermList(BOOL_TYPE, ANY_TYPE, ANY_TYPE), ANY_TYPE);
    quick_create_function(code, "list", create_list__evaluate, TermList(ANY_TYPE), LIST_TYPE);
    quick_create_function(code, "range", range__evaluate, TermList(INT_TYPE), LIST_TYPE);
    quick_create_function(code, "list-apply", list_apply__evaluate, TermList(FUNCTION_TYPE, LIST_TYPE), LIST_TYPE);
    quick_create_function(code, "this-branch", this_branch__evaluate, TermList(), LIST_TYPE);
    quick_create_function(code, "read-text-file", read_text_file__evaluate,
            TermList(STRING_TYPE), STRING_TYPE);
    quick_create_function(code, "write-text-file", write_text_file__evaluate,
            TermList(STRING_TYPE, STRING_TYPE), VOID_TYPE);
}

void initialize()
{
    try {
        bootstrap_kernel();
        initialize_constants();

        // These need to be first:
        initialize_term(KERNEL);
        initialize_term_list(KERNEL);
        initialize_structs(KERNEL);

        // Then everything else:
        initialize_branch(KERNEL);
        initialize_builtin_functions(KERNEL);
        initialize_functions(KERNEL);
        initialize_subroutine(KERNEL);

        // Lastly:
        initialize_bootstrapped_code(KERNEL);
        initialize_external_libraries(KERNEL);

    } catch (errors::CircaError& e)
    {
        std::cout << "An error occured while initializing." << std::endl;
        std::cout << e.message() << std::endl;
        exit(1);
    }
}

} // namespace circa
