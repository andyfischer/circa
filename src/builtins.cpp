// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "importing_macros.h"

#include <iostream>
#include <fstream>

#include "circa.h"
#include "debug_valid_objects.h"
#include "types/color.h"
#include "types/dict.h"
#include "types/list.h"
#include "types/map.h"
#include "types/null.h"

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
Term* GET_NAMESPACE_FIELD = NULL;
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
Term* NAMESPACE_FUNC = NULL;
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

bool FINISHED_BOOTSTRAP = false;

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

void create_primitive_types()
{
    NULL_T = Type::create();
    null_t::setup_type(NULL_T);

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
    FUNCTION_TYPE = create_empty_type(*KERNEL, "Function");
    Type* functionType = &as_type(FUNCTION_TYPE);
    functionType->formatSource = subroutine_t::format_source;
    functionType->checkInvariants = function_t::check_invariants;

    // Initialize Value func
    VALUE_FUNC->type = FUNCTION_TYPE;
    VALUE_FUNC->function = VALUE_FUNC;
    change_type((TaggedValue*)VALUE_FUNC, type_contents(FUNCTION_TYPE));
}

void initialize_primitive_types(Branch& kernel)
{
    STRING_TYPE = create_type(kernel, "string");
    set_type(STRING_TYPE, STRING_T);

    INT_TYPE = create_type(kernel, "int");
    set_type(INT_TYPE, INT_T);

    FLOAT_TYPE = create_type(kernel, "number");
    set_type(FLOAT_TYPE, FLOAT_T);

    DICT_TYPE = create_type(kernel, "Dict");
    set_type(DICT_TYPE, DICT_T);

    BOOL_TYPE = create_type(kernel, "bool");
    set_type(BOOL_TYPE, BOOL_T);

    REF_TYPE = create_type(kernel, "Ref");
    set_type(REF_TYPE, REF_T);

    VOID_TYPE = create_type(kernel, "void");
    set_type(VOID_TYPE, VOID_T);

    LIST_TYPE = create_type(kernel, "List");
    set_type(LIST_TYPE, LIST_T);

    // ANY_TYPE was created in bootstrap_kernel
}

void post_initialize_primitive_types(Branch& kernel)
{
    // Properly setup value() func
    initialize_function(VALUE_FUNC);
    function_t::get_attrs(VALUE_FUNC).outputType = ANY_TYPE;
    function_t::get_evaluate(VALUE_FUNC) = value_function::evaluate;

    ca_assert(function_t::get_output_type(VALUE_FUNC) == ANY_TYPE);
}

void pre_setup_types(Branch& kernel)
{
    // Declare input_placeholder first because it's used while compiling functions
    INPUT_PLACEHOLDER_FUNC = import_function(kernel, NULL, "input_placeholder() -> any");

    // FileSignature is used in some builtin functions
    parse_type(kernel, "type FileSignature { string filename, int time_modified }");

    namespace_function::early_setup(kernel);
}

void initialize_compound_types(Branch& kernel)
{
    type_t::setup_type(TYPE_TYPE);

    Term* set_type = create_compound_type(kernel, "Set");
    set_t::setup_type(type_contents(set_type));

    // LIST_TYPE was created in bootstrap_kernel
    list_t::postponed_setup_type(LIST_TYPE);

    Term* map_type = create_compound_type(kernel, "Map");
    map_t::setup_type(type_contents(map_type));

    branch_ref_t::initialize(kernel);

    Term* styledSourceType = parse_type(kernel, "type StyledSource;");
    styled_source_t::setup_type(&as_type(styledSourceType));

    Term* indexableType = parse_type(kernel, "type Indexable;");
    indexable_t::setup_type(&as_type(indexableType));

    callable_t::setup_type(&as_type(parse_type(kernel, "type Callable;")));
}

void post_setup_functions(Branch& kernel)
{
    // Create vectorized add() functions
    Term* add_v = create_duplicate(kernel, kernel["vectorize_vv"], "add_v");
    set_ref(function_t::get_parameters(add_v), ADD_FUNC);
    Term* add_s = create_duplicate(kernel, kernel["vectorize_vs"], "add_s");
    set_ref(function_t::get_parameters(add_s), ADD_FUNC);

    overloaded_function::append_overload(ADD_FUNC, add_v);
    overloaded_function::append_overload(ADD_FUNC, add_s);

    // Create vectorized sub() functions
    Term* sub_v = create_duplicate(kernel, kernel["vectorize_vv"], "sub_v");
    set_ref(function_t::get_parameters(sub_v), SUB_FUNC);
    Term* sub_s = create_duplicate(kernel, kernel["vectorize_vs"], "sub_s");
    set_ref(function_t::get_parameters(sub_s), SUB_FUNC);

    overloaded_function::append_overload(SUB_FUNC, sub_v);
    overloaded_function::append_overload(SUB_FUNC, sub_s);

    // Create vectorized mult() functions
    Term* mult_v = create_duplicate(kernel, kernel["vectorize_vv"], "mult_v");
    set_ref(function_t::get_parameters(mult_v), MULT_FUNC);
    Term* mult_s = create_duplicate(kernel, kernel["vectorize_vs"], "mult_s");
    set_ref(function_t::get_parameters(mult_s), MULT_FUNC);

    overloaded_function::append_overload(MULT_FUNC, mult_v);
    overloaded_function::append_overload(MULT_FUNC, mult_s);

    // Create vectorized div() function
    Branch& div_overloads = DIV_FUNC->nestedContents;
    Term* div_s = create_duplicate(div_overloads, kernel["vectorize_vs"], "div_s");
    set_ref(function_t::get_parameters(div_s), DIV_FUNC);

    overloaded_function::append_overload(DIV_FUNC, div_s);

    function_t::get_feedback_func(VALUE_FUNC) = UNSAFE_ASSIGN_FUNC;
}

void parse_hosted_types(Branch& kernel)
{
    parse_type(kernel, "type Point { number x, number y }");
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");

    COLOR_TYPE = parse_type(kernel, "type Color { number r, number g, number b, number a }");

    color_t::setup_type(type_contents(COLOR_TYPE));
}

void post_setup_types()
{
    string_t::postponed_setup_type(&as_type(STRING_TYPE));
    ref_t::postponed_setup_type(&as_type(REF_TYPE));
}

void parse_builtin_script(Branch& kernel)
{
    parser::compile(&kernel, parser::statement_list, BUILTIN_SCRIPT_TEXT);
}

} // namespace circa

using namespace circa;

export_func void circa_initialize()
{
    FINISHED_BOOTSTRAP = false;

    create_primitive_types();
    bootstrap_kernel();
    initialize_primitive_types(*KERNEL);
    post_initialize_primitive_types(*KERNEL);
    pre_setup_types(*KERNEL);
    initialize_compound_types(*KERNEL);
    feedback_register_constants(*KERNEL);

    FINISHED_BOOTSTRAP = true;

    setup_builtin_functions(*KERNEL);
    post_setup_functions(*KERNEL);
    parse_hosted_types(*KERNEL);
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
