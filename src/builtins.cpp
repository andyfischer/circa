// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "common_headers.h"

#include <iostream>
#include <fstream>

#include "circa.h"

namespace circa {

// setup_builtin_functions is defined in setup_builtin_functions.cpp
void setup_builtin_functions(Branch&);

Branch* KERNEL = NULL;

Term* ASSIGN_FUNC = NULL;
Term* ADD_FUNC = NULL;
Term* ANNOTATE_TYPE_FUNC = NULL;
Term* APPLY_FEEDBACK = NULL;
Term* AVERAGE_FUNC = NULL;
Term* BRANCH_FUNC = NULL;
Term* BRANCH_TYPE = NULL;
Term* CODEUNIT_TYPE = NULL;
Term* COMMENT_FUNC = NULL;
Term* CONSTANT_TRUE = NULL;
Term* CONSTANT_FALSE = NULL;
Term* COPY_FUNC = NULL;
Term* DESIRED_VALUE_FEEDBACK = NULL;
Term* DIV_FUNC = NULL;
Term* DO_ONCE_FUNC = NULL;
Term* FEEDBACK_FUNC = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FOR_FUNC = NULL;
Term* FUNCTION_TYPE = NULL;
Term* GET_INDEX_FUNC = NULL;
Term* GET_FIELD_FUNC = NULL;
Term* IF_FUNC = NULL;
Term* IF_BLOCK_FUNC = NULL;
Term* IF_EXPR_FUNC = NULL;
Term* INCLUDE_FUNC = NULL;
Term* INPUT_PLACEHOLDER_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_FUNC = NULL;
Term* MAP_TYPE = NULL;
Term* MULT_FUNC = NULL;
Term* NAMESPACE_TYPE = NULL;
Term* NEG_FUNC = NULL;
Term* NOT_FUNC = NULL;
Term* ONE_TIME_ASSIGN_FUNC = NULL;
Term* OVERLOADED_FUNCTION_TYPE = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SET_INDEX_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* TO_REF_FUNC = NULL;
Term* TYPE_TYPE = NULL;
Term* UNKNOWN_FUNCTION = NULL;
Term* UNKNOWN_FIELD_FUNC = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;
Term* VALUE_FUNC = NULL;
Term* VOID_TYPE = NULL;
Term* VOID_PTR_TYPE = NULL;

Term* get_global(std::string name)
{
    if (KERNEL->contains(name))
        return KERNEL->get(name);

    return NULL;
}

void empty_evaluate_function(Term*) { }

void bootstrap_kernel()
{
    // Create the very first building blocks. Most of the building functions in Circa
    // require a few kernel terms to already be defined. So in this function, we
    // create these required terms manually.

    KERNEL = new Branch();

    // Create value function
    VALUE_FUNC = new Term();
    VALUE_FUNC->owningBranch = KERNEL;
    KERNEL->bindName(VALUE_FUNC, "value");

    // Create Type type
    TYPE_TYPE = new Term();
    TYPE_TYPE->owningBranch = KERNEL;
    TYPE_TYPE->function = VALUE_FUNC;
    TYPE_TYPE->type = TYPE_TYPE;
    Type* typeType = new Type();
    TYPE_TYPE->value = typeType;
    typeType->name = "Type";
    typeType->alloc = type_t::alloc;
    typeType->dealloc = type_t::dealloc;
    typeType->assign = type_t::assign;
    typeType->remapPointers = type_t::remap_pointers;
    typeType->toString = type_t::to_string;
    KERNEL->bindName(TYPE_TYPE, "Type");

    // Create Any type
    ANY_TYPE = new Term();
    ANY_TYPE->owningBranch = KERNEL;
    ANY_TYPE->function = VALUE_FUNC;
    ANY_TYPE->type = TYPE_TYPE;
    ANY_TYPE->value = new Type();
    as_type(ANY_TYPE).name = "any";
    KERNEL->bindName(ANY_TYPE, "any");

    // Create Function type
    FUNCTION_TYPE = create_compound_type(*KERNEL, "Function");
    as_type(FUNCTION_TYPE).toString = subroutine_t::to_string;
    as_type(FUNCTION_TYPE).checkInvariants = function_t::check_invariants;

    // Create Branch type
    BRANCH_TYPE = create_compound_type(*KERNEL, "Branch");

    // Initialize Value func
    VALUE_FUNC->type = FUNCTION_TYPE;
    alloc_value(VALUE_FUNC);

    // Initialize List type, it's needed soon
    LIST_TYPE = create_compound_type(*KERNEL, "List");
    as_type(LIST_TYPE).toString = list_t::to_string;
}

void pre_initialize_builtin_types(Branch& kernel)
{
    // Initialize data for Value function
    initialize_function_data(VALUE_FUNC);
    create_value(as_branch(VALUE_FUNC), ANY_TYPE);

    // Declare input_placeholder first because it's used while compiling functions
    INPUT_PLACEHOLDER_FUNC = import_function(kernel, empty_evaluate_function,
            "input_placeholder() : any");
}

void post_setup_builtin_functions(Branch& kernel)
{
    Branch& add_overloads = as_branch(ADD_FUNC);
    Term* add_v = create_duplicate(add_overloads, kernel["vectorize_vv"]);
    create_ref(function_t::get_parameters(add_v), ADD_FUNC);
    rename(add_v, "add_v");
    kernel.bindName(add_v, "add_v");

    Branch& sub_overloads = as_branch(SUB_FUNC);
    Term* sub_v = create_duplicate(sub_overloads, kernel["vectorize_vv"]);
    create_ref(function_t::get_parameters(sub_v), SUB_FUNC);
    rename(sub_v, "sub_v");
    kernel.bindName(sub_v, "sub_v");

    Branch& mult_overloads = as_branch(MULT_FUNC);
    Term* mult_s = create_duplicate(mult_overloads, kernel["vectorize_vs"]);
    create_ref(function_t::get_parameters(mult_s), MULT_FUNC);
    rename(mult_s, "mult_s");
    kernel.bindName(mult_s, "mult_s");

    function_t::get_feedback_func(VALUE_FUNC) = ASSIGN_FUNC;
}

void initialize()
{
    bootstrap_kernel();
    initialize_primitive_types(*KERNEL);
    pre_initialize_builtin_types(*KERNEL);
    setup_builtin_types(*KERNEL);
    feedback_register_constants(*KERNEL);
    setup_primitive_types();
    setup_builtin_functions(*KERNEL);
    post_setup_builtin_functions(*KERNEL);
}

void shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}

} // namespace circa
