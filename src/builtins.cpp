// Copyright 2008 Paul Hodge

#include <iostream>
#include <fstream>

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "builtin_types.h"
#include "cpp_importing.h"
#include "function.h"
#include "importing.h"
#include "list.h"
#include "runtime.h"
#include "term.h"
#include "token_stream.h"
#include "type.h"
#include "values.h"

namespace circa {

// setup_builtin_functions is defined in setup_builtin_functions.cpp
void setup_builtin_functions(Branch&);

Branch* KERNEL = NULL;
Term* VALUE_FUNCTION_GENERATOR = NULL;
Term* VALUE_FUNCTION_FEEDBACK_ASSIGN = NULL;
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
Term* VOID_PTR_TYPE = NULL;
Term* REFERENCE_TYPE = NULL;
Term* LIST_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* VAR_INT = NULL;
Term* VAR_FLOAT = NULL;
Term* VAR_STRING = NULL;
Term* VAR_BOOL = NULL;
Term* CONSTANT_0 = NULL;
Term* CONSTANT_1 = NULL;
Term* CONSTANT_2 = NULL;
Term* CONSTANT_TRUE = NULL;
Term* CONSTANT_FALSE = NULL;
Term* UNKNOWN_FUNCTION = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* APPLY_FEEDBACK = NULL;
Term* ADD_FUNC = NULL;
Term* MULT_FUNC = NULL;
Term* ALIAS_FUNC = NULL;
Term* COMMENT_FUNC = NULL;
Term* INT_TO_FLOAT_FUNC = NULL;
Term* COPY_FUNC = NULL;
Term* IF_STATEMENT = NULL;

void empty_evaluate_function(Term*) { }
void empty_alloc_function(Term*) { }

void value_function_generator(Term* caller)
{
    assert(caller->input(0) != NULL);

    Function& output = as_function(caller);
    Type& type = as_type(caller->input(0));
    output.name = "value-" + type.name;
    output.outputType = caller->input(0);
    output.pureFunction = false;
    output.evaluate = empty_evaluate_function;
    output.feedbackPropogateFunction = VALUE_FUNCTION_FEEDBACK_ASSIGN;
}

Term* get_global(std::string name)
{
    if (KERNEL->containsName(name))
        return KERNEL->getNamed(name);

    return NULL;
}

void bootstrap_kernel()
{
    // This is a crazy function. We need to create the 5 core functions in our system,
    // all of which need to reference each other or themselves.
    //
    // Here is what we need to create:
    //
    // var-function-generator(Type) -> Function
    //    Given a type, returns a function which is a 'plain value' function. This term
    //    has function const-Function.
    // const-Type() -> Type
    //    Function which returns a Type value. This is created by var-function-generator
    // const-Function() -> Function
    //    Function which returns a Function value. This is created by var-function-generator.
    // Type Function
    //    Stores the Type object for functions. This term has function const-Type
    // Type Type
    //    Stores the Type object for types. This term has function const-Type

    KERNEL = new Branch();

    // Create var-function-generator function
    VALUE_FUNCTION_GENERATOR = new Term();
    VALUE_FUNCTION_GENERATOR->owningBranch = KERNEL;
    VALUE_FUNCTION_GENERATOR->value = new Function();
    as_function(VALUE_FUNCTION_GENERATOR).name = "value-function-generator";
    as_function(VALUE_FUNCTION_GENERATOR).pureFunction = true;
    as_function(VALUE_FUNCTION_GENERATOR).evaluate = value_function_generator;
    KERNEL->bindName(VALUE_FUNCTION_GENERATOR, "value-function-generator");

    // Create const-Type function
    Term* constTypeFunc = new Term();
    constTypeFunc->owningBranch = KERNEL;
    constTypeFunc->function = VALUE_FUNCTION_GENERATOR;
    constTypeFunc->value = new Function();
    as_function(constTypeFunc).name = "const-Type";
    as_function(constTypeFunc).pureFunction = true;

    // Create Type type
    TYPE_TYPE = new Term();
    TYPE_TYPE->owningBranch = KERNEL;
    TYPE_TYPE->function = constTypeFunc;
    TYPE_TYPE->type = TYPE_TYPE;
    TYPE_TYPE->value = new Type();

    as_type(TYPE_TYPE).alloc = cpp_importing::templated_alloc<Type>;
    as_type(TYPE_TYPE).dealloc = cpp_importing::templated_dealloc<Type>;
    as_type(TYPE_TYPE).copy = Type::type_copy;
    as_type(TYPE_TYPE).remapPointers = Type::typeRemapPointers;
    as_type(TYPE_TYPE).visitPointers = Type::typeVisitPointers;
    as_type(TYPE_TYPE).startPointerIterator = Type::typeStartPointerIterator;
    KERNEL->bindName(TYPE_TYPE, "Type");

    // Implant the Type type
    set_input(constTypeFunc, 0, TYPE_TYPE);
    as_function(VALUE_FUNCTION_GENERATOR).inputTypes.setAt(0, TYPE_TYPE);
    as_function(constTypeFunc).outputType = TYPE_TYPE;

    // Create const-Function function
    Term* constFuncFunc = new Term();
    constFuncFunc->owningBranch = KERNEL;
    constFuncFunc->function = VALUE_FUNCTION_GENERATOR;
    constFuncFunc->value = new Function();
    as_function(constFuncFunc).name = "const-Function";
    as_function(constFuncFunc).pureFunction = true;
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Implant const-Function
    VALUE_FUNCTION_GENERATOR->function = constFuncFunc;

    // Create Function type
    FUNCTION_TYPE = new Term();
    FUNCTION_TYPE->owningBranch = KERNEL;
    FUNCTION_TYPE->function = constTypeFunc;
    FUNCTION_TYPE->type = TYPE_TYPE;
    FUNCTION_TYPE->value = alloc_from_type(TYPE_TYPE);
    as_type(FUNCTION_TYPE).name = "Function";
    as_type(FUNCTION_TYPE).alloc = cpp_importing::templated_alloc<Function>;
    as_type(FUNCTION_TYPE).dealloc = cpp_importing::templated_dealloc<Function>;
    as_type(FUNCTION_TYPE).copy = Function::copy;
    as_type(FUNCTION_TYPE).remapPointers = Function::remapPointers;
    as_type(FUNCTION_TYPE).visitPointers = Function::visitPointers;
    KERNEL->bindName(FUNCTION_TYPE, "Function");

    // Implant Function type
    set_input(VALUE_FUNCTION_GENERATOR, 0, TYPE_TYPE);
    set_input(constFuncFunc, 0, FUNCTION_TYPE);
    VALUE_FUNCTION_GENERATOR->type = FUNCTION_TYPE;
    constFuncFunc->type = FUNCTION_TYPE;
    constTypeFunc->type = FUNCTION_TYPE;
    as_function(VALUE_FUNCTION_GENERATOR).outputType = FUNCTION_TYPE;
    as_function(constFuncFunc).outputType = FUNCTION_TYPE;

    // Don't let these terms get updated
    VALUE_FUNCTION_GENERATOR->needsUpdate = false;
    constFuncFunc->needsUpdate = false;
    constTypeFunc->needsUpdate = false;
    FUNCTION_TYPE->needsUpdate = false;
    TYPE_TYPE->needsUpdate = false;
}

namespace var_function {

    void feedback_assign(Term* caller)
    {
        Term* target = caller->input(0);
        Term* desired = caller->input(1);

        copy_value(desired, target);
    }
}

void initialize_constants()
{
    BRANCH_TYPE = import_type<Branch>(*KERNEL, "Branch");
    as_type(BRANCH_TYPE).copy = Branch::copy;
    as_type(BRANCH_TYPE).remapPointers = Branch::hosted_remap_pointers;
    as_type(BRANCH_TYPE).visitPointers = Branch::hosted_visit_pointers;
    as_type(BRANCH_TYPE).startPointerIterator = Branch::start_pointer_iterator;

    LIST_TYPE = import_type<List>(*KERNEL, "List");
    as_type(LIST_TYPE).toString = List::to_string;

    import_type<Dictionary>(*KERNEL, "Dictionary");

    VAR_INT = get_value_function(INT_TYPE);
    VAR_FLOAT = get_value_function(FLOAT_TYPE);
    VAR_STRING = get_value_function(STRING_TYPE);
    VAR_BOOL = get_value_function(BOOL_TYPE);

    CONSTANT_0 = float_value(*KERNEL, 0);
    CONSTANT_1 = float_value(*KERNEL, 1);
    CONSTANT_2 = float_value(*KERNEL, 2);

    CONSTANT_TRUE = apply_function(KERNEL, BOOL_TYPE, RefList());
    as_bool(CONSTANT_TRUE) = true;
    CONSTANT_TRUE->stealingOk = false;
    KERNEL->bindName(CONSTANT_TRUE, "true");

    CONSTANT_FALSE = apply_function(KERNEL, BOOL_TYPE, RefList());
    as_bool(CONSTANT_FALSE) = false;
    CONSTANT_FALSE->stealingOk = false;
    KERNEL->bindName(CONSTANT_FALSE, "false");

    Term* pi = apply_function(KERNEL, FLOAT_TYPE, RefList());
    as_float(pi) = 3.14159;
    KERNEL->bindName(pi, "PI");

    Term* tokenStreamType = 
        import_type<TokenStream>(*KERNEL, "TokenStream");
    register_cpp_toString<TokenStream>(tokenStreamType);
}

void initialize_builtin_functions(Branch* kernel)
{
    setup_builtin_functions(*kernel);

    ADD_FUNC = kernel->getNamed("add");
    MULT_FUNC = kernel->getNamed("mult");
    COPY_FUNC = kernel->getNamed("copy");

    assert(ADD_FUNC != NULL);
    assert(MULT_FUNC != NULL);

    VALUE_FUNCTION_FEEDBACK_ASSIGN = import_function(*kernel,
        var_function::feedback_assign, "var-function-feedback-assign(any,any)");

    as_function(VAR_INT).feedbackPropogateFunction = VALUE_FUNCTION_FEEDBACK_ASSIGN;
    as_function(VAR_FLOAT).feedbackPropogateFunction = VALUE_FUNCTION_FEEDBACK_ASSIGN;
    as_function(VAR_BOOL).feedbackPropogateFunction = VALUE_FUNCTION_FEEDBACK_ASSIGN;
    as_function(VAR_STRING).feedbackPropogateFunction = VALUE_FUNCTION_FEEDBACK_ASSIGN;
}

void initialize()
{
    bootstrap_kernel();
    initialize_builtin_types(*KERNEL);
    initialize_constants();
    initialize_builtin_functions(KERNEL);
}

void shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}

} // namespace circa
