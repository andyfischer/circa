// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "importing_macros.h"

#include <iostream>
#include <fstream>

#include "circa.h"
#include "debug_valid_objects.h"
#include "types/dict.h"
#include "types/list.h"

namespace circa {

// setup_functions is defined in generated/setup_builtin_functions.cpp
void setup_builtin_functions(Branch&);

extern "C" {

// BUILTIN_SCRIPT_TEXT is defined in generated/builtin_script_text.cpp
extern const char* BUILTIN_SCRIPT_TEXT;

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
Term* GET_INDEX_FROM_BRANCH_FUNC = NULL;
Term* GET_FIELD_FUNC = NULL;
Term* IF_FUNC = NULL;
Term* IF_BLOCK_FUNC = NULL;
Term* COND_FUNC = NULL;
Term* INCLUDE_FUNC = NULL;
Term* INPUT_PLACEHOLDER_FUNC = NULL;
Term* JOIN_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_FUNC = NULL;
Term* MULT_FUNC = NULL;
Term* NAMESPACE_TYPE = NULL;
Term* NEG_FUNC = NULL;
Term* NOT_FUNC = NULL;
Term* ONE_TIME_ASSIGN_FUNC = NULL;
Term* OVERLOADED_FUNCTION_FUNC = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SET_INDEX_FUNC = NULL;
Term* STATEFUL_VALUE_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* RANGE_FUNC = NULL;
Term* REF_FUNC = NULL;
Term* RETURN_FUNC = NULL;
Term* UNKNOWN_FUNCTION = NULL;
Term* UNKNOWN_FIELD_FUNC = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;
Term* UNSAFE_ASSIGN_FUNC = NULL;
Term* VALUE_FUNC = NULL;

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* DICT_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* BRANCH_TYPE = NULL;
Term* CODE_TYPE = NULL;
Term* COLOR_TYPE = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* FUNCTION_ATTRS_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* VOID_TYPE = NULL;

} // extern "C"

TypeRef TYPE_T;
TypeRef BOOL_T;
TypeRef DICT_T;
TypeRef FLOAT_T;
TypeRef INT_T;
TypeRef NULL_T;
TypeRef STRING_T;
TypeRef REF_T;
TypeRef LIST_T;
TypeRef VOID_T;

Branch& kernel()
{
    return *KERNEL;
}

Term* get_global(std::string name)
{
    if (KERNEL->contains(name))
        return KERNEL->get(name);

    return NULL;
}

CA_FUNCTION(empty_evaluate_function) {}

void empty_bytecode_generation(bytecode::WriteContext* context, Term* term) {}

namespace null_t {
    std::string toString(TaggedValue* value) { return "null";}
}

void create_types()
{
    NULL_T = Type::create();
    NULL_T->name = "null";
    NULL_T->toString = null_t::toString;

    DICT_T = Type::create();
    dict_t::setup_type(DICT_T);

    STRING_T = Type::create();
    string_t::setup_type(STRING_T);

    INT_T = Type::create();
    int_t::setup_type(INT_T);

    FLOAT_T = Type::create();
    float_t::setup_type(FLOAT_T);

    BOOL_T = Type::create();
    bool_t::setup_type(BOOL_T);

    REF_T = Type::create();
    ref_t::setup_type(REF_T);

    LIST_T = Type::create();
    list_t::setup_type(LIST_T);

    VOID_T = Type::create();
    void_t::setup_type(VOID_T);
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
    TYPE_T = Type::create();
    TYPE_TYPE->value_type = TYPE_T;
    TYPE_TYPE->value_data.ptr = TYPE_T;
    TYPE_T->name = "Type";
    TYPE_T->initialize = type_t::initialize;
    TYPE_T->release = type_t::release;
    TYPE_T->copy = type_t::copy;
    TYPE_T->remapPointers = type_t::remap_pointers;
    TYPE_T->formatSource = type_t::formatSource;
    KERNEL->bindName(TYPE_TYPE, "Type");

    // Create Any type
    ANY_TYPE = KERNEL->appendNew();
    ANY_TYPE->function = VALUE_FUNC;
    ANY_TYPE->type = TYPE_TYPE;
    change_type(ANY_TYPE, TYPE_T);
    as_type(ANY_TYPE).name = "any";
    as_type(ANY_TYPE).toString = any_t::to_string;
    as_type(ANY_TYPE).isSubtype = any_t::matches_type;
    as_type(ANY_TYPE).cast = any_t::cast;
    KERNEL->bindName(ANY_TYPE, "any");

    // Create Branch type
    BRANCH_TYPE = create_branch_based_type(*KERNEL, "Branch");

    // Create FunctionAttrs type
    FUNCTION_ATTRS_TYPE = KERNEL->appendNew();
    FUNCTION_ATTRS_TYPE->function = VALUE_FUNC;
    FUNCTION_ATTRS_TYPE->type = TYPE_TYPE;
    change_type(FUNCTION_ATTRS_TYPE, TYPE_T);
    as_type(FUNCTION_ATTRS_TYPE).name = "FunctionAttrs";
    as_type(FUNCTION_ATTRS_TYPE).initialize = function_attrs_t::initialize;
    as_type(FUNCTION_ATTRS_TYPE).copy = function_attrs_t::copy;
    as_type(FUNCTION_ATTRS_TYPE).release = function_attrs_t::release;
    KERNEL->bindName(FUNCTION_ATTRS_TYPE, "FunctionAttrs");
    ca_assert(is_type(FUNCTION_ATTRS_TYPE));

    // Create Function type
    FUNCTION_TYPE = create_branch_based_type(*KERNEL, "Function");
    Type* functionType = &as_type(FUNCTION_TYPE);
    functionType->formatSource = subroutine_t::format_source;
    functionType->checkInvariants = function_t::check_invariants;

    // Initialize Value func
    VALUE_FUNC->type = FUNCTION_TYPE;
    VALUE_FUNC->function = VALUE_FUNC;
    change_type((TaggedValue*)VALUE_FUNC, (Type*)BRANCH_TYPE->value_data.ptr);

    // Initialize List type, it's needed soon
}

void post_initialize_primitive_types(Branch& kernel)
{
    // Properly setup value() func
    initialize_function(VALUE_FUNC);
    function_t::get_attrs(VALUE_FUNC).outputType = ANY_TYPE;
    function_t::get_evaluate(VALUE_FUNC) = value_function::evaluate;

    ca_assert(function_t::get_output_type(VALUE_FUNC) == ANY_TYPE);
}

void pre_initialize_types(Branch& kernel)
{
    // Declare input_placeholder first because it's used while compiling functions
    INPUT_PLACEHOLDER_FUNC = import_function(kernel, empty_evaluate_function,
            "input_placeholder() -> any");

    function_t::get_attrs(INPUT_PLACEHOLDER_FUNC).writeBytecode = empty_bytecode_generation;

    // FileSignature is used in some builtin functions
    parse_type(kernel, "type FileSignature { string filename, int time_modified }");
}

void post_setup_functions(Branch& kernel)
{
    // Create vectorized add() functions
    Term* add_v = create_duplicate(kernel, kernel["vectorize_vv"], "add_v");
    make_ref(function_t::get_parameters(add_v), ADD_FUNC);
    Term* add_s = create_duplicate(kernel, kernel["vectorize_vs"], "add_s");
    make_ref(function_t::get_parameters(add_s), ADD_FUNC);

    overloaded_function::append_overload(ADD_FUNC, add_v);
    overloaded_function::append_overload(ADD_FUNC, add_s);

    // Create vectorized sub() functions
    Term* sub_v = create_duplicate(kernel, kernel["vectorize_vv"], "sub_v");
    make_ref(function_t::get_parameters(sub_v), SUB_FUNC);
    Term* sub_s = create_duplicate(kernel, kernel["vectorize_vs"], "sub_s");
    make_ref(function_t::get_parameters(sub_s), SUB_FUNC);

    overloaded_function::append_overload(SUB_FUNC, sub_v);
    overloaded_function::append_overload(SUB_FUNC, sub_s);

    // Create vectorized mult() functions
    Term* mult_v = create_duplicate(kernel, kernel["vectorize_vv"], "mult_v");
    make_ref(function_t::get_parameters(mult_v), MULT_FUNC);
    Term* mult_s = create_duplicate(kernel, kernel["vectorize_vs"], "mult_s");
    make_ref(function_t::get_parameters(mult_s), MULT_FUNC);

    overloaded_function::append_overload(MULT_FUNC, mult_v);
    overloaded_function::append_overload(MULT_FUNC, mult_s);

    // Create vectorized div() function
    Branch& div_overloads = DIV_FUNC->nestedContents;
    Term* div_s = create_duplicate(div_overloads, kernel["vectorize_vs"], "div_s");
    make_ref(function_t::get_parameters(div_s), DIV_FUNC);

    overloaded_function::append_overload(DIV_FUNC, div_s);

    function_t::get_feedback_func(VALUE_FUNC) = UNSAFE_ASSIGN_FUNC;
}

void parse_builtin_script(Branch& kernel)
{
    parser::compile(&kernel, parser::statement_list, BUILTIN_SCRIPT_TEXT);
}

} // namespace circa

using namespace circa;

export_func void circa_initialize()
{
    create_types();
    bootstrap_kernel();
    initialize_primitive_types(*KERNEL);
    post_initialize_primitive_types(*KERNEL);
    pre_initialize_types(*KERNEL);
    setup_types(*KERNEL);
    feedback_register_constants(*KERNEL);
    setup_builtin_functions(*KERNEL);
    post_setup_functions(*KERNEL);
    parse_types(*KERNEL);
    post_setup_types();
    type_initialize_kernel(*KERNEL);
    initialize_kernel_documentation(*KERNEL);
    parse_builtin_script(*KERNEL);
}

export_func void circa_shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}
