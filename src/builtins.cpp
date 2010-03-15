// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <iostream>
#include <fstream>

#include "circa.h"

namespace circa {

// setup_builtin_functions is defined in generated/setup_builtin_functions.cpp
void setup_builtin_functions(Branch&);

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
Term* GET_FIELD_FUNC = NULL;
Term* IF_FUNC = NULL;
Term* IF_BLOCK_FUNC = NULL;
Term* COND_FUNC = NULL;
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

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* BRANCH_TYPE = NULL;
Term* BRANCH_REF_TYPE = NULL;
Term* CODE_TYPE = NULL;
Term* COLOR_TYPE = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* FUNCTION_ATTRS_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* OVERLOADED_FUNCTION_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* VOID_TYPE = NULL;

// New style: Type* pointers for builtins
Type* BOOL_T = NULL;
Type* FLOAT_T = NULL;
Type* INT_T = NULL;
Type* NULL_T = NULL;
Type* STRING_T = NULL;
Type* REF_T = NULL;

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

    STRING_T = new Type();
    STRING_T->name = "string";
    STRING_T->initialize = string_t::initialize;
    STRING_T->release = string_t::release;
    STRING_T->assign = string_t::assign;
    STRING_T->equals = string_t::equals;
    STRING_T->toString = string_t::to_string;

    INT_T = new Type();
    INT_T->name = "int";
    INT_T->equals = int_t::equals;
    INT_T->toString = int_t::to_string;

    FLOAT_T = new Type();
    FLOAT_T->name = "float";
    FLOAT_T->cast = float_t::cast;
    FLOAT_T->castPossible = float_t::cast_possible;
    FLOAT_T->equals = float_t::equals;
    FLOAT_T->toString = float_t::to_string;

    BOOL_T = new Type();
    BOOL_T->name = "bool";
    BOOL_T->toString = bool_t::to_string;

    REF_T = new Type();
    REF_T->name = "ref";
    REF_T->remapPointers = Ref::remap_pointers;
    REF_T->toString = ref_t::to_string;
    REF_T->initialize = ref_t::initialize;
    REF_T->assign = ref_t::assign;
    REF_T->equals = ref_t::equals;
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
    TYPE_TYPE->value_type = typeType;
    TYPE_TYPE->value_data.ptr = typeType;
    typeType->name = "Type";
    typeType->initialize = type_t::initialize;
    typeType->assign = type_t::assign;
    typeType->remapPointers = type_t::remap_pointers;
    typeType->toString = type_t::to_string;
    KERNEL->bindName(TYPE_TYPE, "Type");

    // Create Any type
    ANY_TYPE = KERNEL->appendNew();
    ANY_TYPE->function = VALUE_FUNC;
    ANY_TYPE->type = TYPE_TYPE;
    make_type(ANY_TYPE, new Type());
    type_t::get_name(ANY_TYPE) = "any";
    KERNEL->bindName(ANY_TYPE, "any");

    // Create Branch type
    BRANCH_TYPE = create_compound_type(*KERNEL, "Branch");

    // Create FunctionAttrs type
    FUNCTION_ATTRS_TYPE = KERNEL->appendNew();
    FUNCTION_ATTRS_TYPE->function = VALUE_FUNC;
    FUNCTION_ATTRS_TYPE->type = TYPE_TYPE;
    Type* functionAttrsType = new Type();
    FUNCTION_ATTRS_TYPE->value_type = typeType;
    FUNCTION_ATTRS_TYPE->value_data.ptr = functionAttrsType;
    functionAttrsType->initialize = function_attrs_t::initialize;
    functionAttrsType->assign = function_attrs_t::assign;
    functionAttrsType->release = function_attrs_t::release;
    KERNEL->bindName(FUNCTION_ATTRS_TYPE, "FunctionAttrs");
    assert(is_type(FUNCTION_ATTRS_TYPE));

    // Create Function type
    FUNCTION_TYPE = create_compound_type(*KERNEL, "Function");
    Type* functionType = &as_type(FUNCTION_TYPE);
    functionType->toString = subroutine_t::to_string;
    functionType->checkInvariants = function_t::check_invariants;

    // Initialize Value func
    VALUE_FUNC->type = FUNCTION_TYPE;
    VALUE_FUNC->function = VALUE_FUNC;
    change_type((TaggedValue*)VALUE_FUNC, (Type*)BRANCH_TYPE->value_data.ptr);

    // Initialize List type, it's needed soon
    LIST_TYPE = create_compound_type(*KERNEL, "List");
    Type* listType = &as_type(LIST_TYPE);
    listType->toString = list_t::to_string;
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
    make_ref(function_t::get_parameters(add_v), ADD_FUNC);
    Term* add_s = create_duplicate(kernel, kernel["vectorize_vs"], "add_s");
    make_ref(function_t::get_parameters(add_s), ADD_FUNC);

    // Add to add() overloads
    create_ref(as_branch(ADD_FUNC), add_v);
    create_ref(as_branch(ADD_FUNC), add_s);

    Term* sub_v = create_duplicate(kernel, kernel["vectorize_vv"], "sub_v");
    make_ref(function_t::get_parameters(sub_v), SUB_FUNC);
    Term* sub_s = create_duplicate(kernel, kernel["vectorize_vs"], "sub_s");
    make_ref(function_t::get_parameters(sub_s), SUB_FUNC);

    // Add to sub() overloads
    create_ref(as_branch(SUB_FUNC), sub_v);
    create_ref(as_branch(SUB_FUNC), sub_s);

    Term* mult_v = create_duplicate(kernel, kernel["vectorize_vv"], "mult_v");
    make_ref(function_t::get_parameters(mult_v), MULT_FUNC);
    Term* mult_s = create_duplicate(kernel, kernel["vectorize_vs"], "mult_s");
    make_ref(function_t::get_parameters(mult_s), MULT_FUNC);

    // Add to mult() overloads
    create_ref(as_branch(MULT_FUNC), mult_v);
    create_ref(as_branch(MULT_FUNC), mult_s);

    Branch& div_overloads = as_branch(DIV_FUNC);
    Term* div_s = create_duplicate(div_overloads, kernel["vectorize_vs"], "div_s");
    make_ref(function_t::get_parameters(div_s), DIV_FUNC);

    // Add to div() overloads
    create_ref(as_branch(DIV_FUNC), div_s);

    function_t::get_feedback_func(VALUE_FUNC) = ASSIGN_FUNC;
    hide_from_docs(VALUE_FUNC);
}

void parse_builtin_script(Branch& kernel)
{
    parser::compile(&kernel, parser::statement_list, BUILTIN_SCRIPT_TEXT);
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
    parse_builtin_script(*KERNEL);
}

void shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}

} // namespace circa
