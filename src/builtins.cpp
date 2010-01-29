// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <iostream>
#include <fstream>

#include "circa.h"

namespace circa {

// setup_builtin_functions is defined in setup_builtin_functions.cpp
void setup_builtin_functions(Branch&);

Branch* KERNEL = NULL;

Term* ALIAS_FUNC = NULL;
Term* ASSIGN_FUNC = NULL;
Term* ADD_FUNC = NULL;
Term* APPLY_FEEDBACK = NULL;
Term* AVERAGE_FUNC = NULL;
Term* BRANCH_FUNC = NULL;
Term* CAST_FUNC = NULL;
Term* COMMENT_FUNC = NULL;
Term* COPY_FUNC = NULL;
Term* DESIRED_VALUE_FEEDBACK = NULL;
Term* DISCARD_FUNC = NULL;
Term* DIV_FUNC = NULL;
Term* DO_ONCE_FUNC = NULL;
Term* FEEDBACK_FUNC = NULL;
Term* FREEZE_FUNC = NULL;
Term* FOR_FUNC = NULL;
Term* GET_INDEX_FUNC = NULL;
Term* GET_FIELD_FUNC = NULL;
Term* IF_FUNC = NULL;
Term* IF_BLOCK_FUNC = NULL;
Term* IF_EXPR_FUNC = NULL;
Term* INCLUDE_FUNC = NULL;
Term* INPUT_PLACEHOLDER_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_FUNC = NULL;
Term* MULT_FUNC = NULL;
Term* NAMESPACE_TYPE = NULL;
Term* NEG_FUNC = NULL;
Term* NOT_FUNC = NULL;
Term* ONE_TIME_ASSIGN_FUNC = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SET_INDEX_FUNC = NULL;
Term* STATEFUL_VALUE_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* TO_REF_FUNC = NULL;
Term* UNKNOWN_FUNCTION = NULL;
Term* UNKNOWN_FIELD_FUNC = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;
Term* VALUE_FUNC = NULL;

Term* BRANCH_TYPE = NULL;
Term* BRANCH_REF_TYPE = NULL;
Term* CODE_TYPE = NULL;
Term* COLOR_TYPE = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* OVERLOADED_FUNCTION_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* VOID_TYPE = NULL;

// New style: Type* pointers for builtins
Type* NULL_T = NULL;

Term* get_global(std::string name)
{
    if (KERNEL->contains(name))
        return KERNEL->get(name);

    return NULL;
}

void empty_evaluate_function(EvalContext*, Term*) { }
void create_builtin_types()
{
    NULL_T = new Type();
    NULL_T->name = "null";
}

void bootstrap_kernel()
{
    // Create the very first building blocks. Most of the building functions in Circa
    // require a few kernel terms to already be defined. So in this function, we
    // create these required terms manually.

    KERNEL = new Branch();

    // Create value function
    VALUE_FUNC = KERNEL->appendNew();
    KERNEL->bindName(VALUE_FUNC, "value");

    // Create Type type
    TYPE_TYPE = KERNEL->appendNew();
    TYPE_TYPE->function = VALUE_FUNC;
    TYPE_TYPE->type = TYPE_TYPE;
    Type* typeType = new Type();
    set_type_value(TYPE_TYPE, typeType);
    typeType->name = "Type";
    typeType->initialize = type_t::initialize;
    typeType->assign2 = type_t::assign;
    typeType->remapPointers = type_t::remap_pointers;
    typeType->toString = type_t::to_string;
    KERNEL->bindName(TYPE_TYPE, "Type");

    // Create Any type
    ANY_TYPE = KERNEL->appendNew();
    ANY_TYPE->function = VALUE_FUNC;
    ANY_TYPE->type = TYPE_TYPE;
    set_type_value(ANY_TYPE, new Type());
    type_t::get_name(ANY_TYPE) = "any";
    KERNEL->bindName(ANY_TYPE, "any");

    // Create Branch type
    BRANCH_TYPE = create_compound_type(*KERNEL, "Branch");

    // Create Function type
    FUNCTION_TYPE = create_compound_type(*KERNEL, "Function");
    type_t::get_to_string_func(FUNCTION_TYPE) = subroutine_t::to_string;
    type_t::get_check_invariants_func(FUNCTION_TYPE)= function_t::check_invariants;

    // Initialize Value func
    VALUE_FUNC->type = FUNCTION_TYPE;
    VALUE_FUNC->function = VALUE_FUNC;
    alloc_value(VALUE_FUNC);

    // Initialize List type, it's needed soon
    LIST_TYPE = create_compound_type(*KERNEL, "List");
    type_t::get_to_string_func(LIST_TYPE) = list_t::to_string;
}

void post_initialize_primitive_types(Branch& kernel)
{
    // Initialize a proper prototype for Function type
    initialize_function_prototype(type_t::get_prototype(FUNCTION_TYPE));

    // Value function was created before we had a prototype
    initialize_function_prototype(as_branch(VALUE_FUNC));
    create_value(as_branch(VALUE_FUNC), ANY_TYPE, "#out");

    assert(function_t::get_output_type(VALUE_FUNC) == ANY_TYPE);
}

void pre_initialize_builtin_types(Branch& kernel)
{
    // Declare input_placeholder first because it's used while compiling functions
    INPUT_PLACEHOLDER_FUNC = import_function(kernel, empty_evaluate_function,
            "input_placeholder() -> any");

    // FileSignature is used in some builtin functions
    parse_type(kernel, "type FileSignature { string filename, int time_modified }");
}

void post_setup_builtin_functions(Branch& kernel)
{
    Term* add_v = create_duplicate(kernel, kernel["vectorize_vv"], "add_v");
    create_ref(function_t::get_parameters(add_v), ADD_FUNC);
    Term* add_s = create_duplicate(kernel, kernel["vectorize_vs"], "add_s");
    create_ref(function_t::get_parameters(add_s), ADD_FUNC);

    // Add to add() overloads
    create_ref(as_branch(ADD_FUNC), add_v);
    create_ref(as_branch(ADD_FUNC), add_s);

    Term* sub_v = create_duplicate(kernel, kernel["vectorize_vv"], "sub_v");
    create_ref(function_t::get_parameters(sub_v), SUB_FUNC);
    Term* sub_s = create_duplicate(kernel, kernel["vectorize_vs"], "sub_s");
    create_ref(function_t::get_parameters(sub_s), SUB_FUNC);

    // Add to sub() overloads
    create_ref(as_branch(SUB_FUNC), sub_v);
    create_ref(as_branch(SUB_FUNC), sub_s);

    Term* mult_v = create_duplicate(kernel, kernel["vectorize_vv"], "mult_v");
    create_ref(function_t::get_parameters(mult_v), MULT_FUNC);
    Term* mult_s = create_duplicate(kernel, kernel["vectorize_vs"], "mult_s");
    create_ref(function_t::get_parameters(mult_s), MULT_FUNC);

    // Add to mult() overloads
    create_ref(as_branch(MULT_FUNC), mult_v);
    create_ref(as_branch(MULT_FUNC), mult_s);

    Branch& div_overloads = as_branch(DIV_FUNC);
    Term* div_s = create_duplicate(div_overloads, kernel["vectorize_vs"], "div_s");
    create_ref(function_t::get_parameters(div_s), DIV_FUNC);

    // Add to div() overloads
    create_ref(as_branch(DIV_FUNC), div_s);

    function_t::get_feedback_func(VALUE_FUNC) = ASSIGN_FUNC;
    hide_from_docs(VALUE_FUNC);
}

void initialize()
{
    create_builtin_types();
    bootstrap_kernel();
    initialize_primitive_types(*KERNEL);
    post_initialize_primitive_types(*KERNEL);
    pre_initialize_builtin_types(*KERNEL);
    setup_builtin_types(*KERNEL);
    feedback_register_constants(*KERNEL);
    setup_builtin_functions(*KERNEL);
    post_setup_builtin_functions(*KERNEL);
    parse_builtin_types(*KERNEL);
    post_setup_primitive_types();
    initialize_kernel_documentation(*KERNEL);
}

void shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}

} // namespace circa
