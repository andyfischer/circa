// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "dict.h"
#include "evaluation.h"
#include "filesystem.h"
#include "function.h"
#include "kernel.h"
#include "gc.h"
#include "importing.h"
#include "importing_macros.h"
#include "parser.h"
#include "static_checking.h"
#include "term.h"
#include "type.h"

#include "types/any.h"
#include "types/bool.h"
#include "types/callable.h"
#include "types/color.h"
#include "types/common.h"
#include "types/eval_context.h"
#include "types/float.h"
#include "types/handle.h"
#include "types/hashtable.h"
#include "types/indexable.h"
#include "types/int.h"
#include "types/ref.h"
#include "types/set.h"
#include "types/string.h"
#include "types/symbol.h"
#include "types/void.h"

namespace circa {

Branch* KERNEL = NULL;

// STDLIB_CA_TEXT is defined in generated/stdlib_script_text.cpp
extern "C" {
    extern const char* STDLIB_CA_TEXT;
}

// setup_functions is defined in generated/setup_builtin_functions.cpp
void setup_builtin_functions(Branch*);

bool STATIC_INITIALIZATION_FINISHED = false;
bool FINISHED_BOOTSTRAP = false;
bool SHUTTING_DOWN = false;

Term* ADD_FUNC = NULL;
Term* ADDITIONAL_OUTPUT_FUNC = NULL;
Term* ALIAS_FUNC = NULL;
Term* ASSIGN_FUNC = NULL;
Term* APPLY_FEEDBACK = NULL;
Term* AVERAGE_FUNC = NULL;
Term* BRANCH_FUNC = NULL;
Term* BRANCH_UNEVALUATED_FUNC = NULL;
Term* BREAK_FUNC = NULL;
Term* CASE_FUNC = NULL;
Term* CAST_FUNC = NULL;
Term* COMMENT_FUNC = NULL;
Term* CONTINUE_FUNC = NULL;
Term* COPY_FUNC = NULL;
Term* DEFAULT_CASE_FUNC = NULL;
Term* DESIRED_VALUE_FEEDBACK = NULL;
Term* DISCARD_FUNC = NULL;
Term* DIV_FUNC = NULL;
Term* DO_ONCE_FUNC = NULL;
Term* ERRORED_FUNC = NULL;
Term* EXTRA_OUTPUT_FUNC = NULL;
Term* FEEDBACK_FUNC = NULL;
Term* FINISH_MINOR_BRANCH_FUNC = NULL;
Term* FREEZE_FUNC = NULL;
Term* FOR_FUNC = NULL;
Term* GET_FIELD_FUNC = NULL;
Term* GET_INDEX_FUNC = NULL;
Term* GET_INDEX_FROM_BRANCH_FUNC = NULL;
Term* GET_STATE_FIELD_FUNC = NULL;
Term* IF_BLOCK_FUNC = NULL;
Term* COND_FUNC = NULL;
Term* INCLUDE_FUNC = NULL;
Term* INPUT_PLACEHOLDER_FUNC = NULL;
Term* INSTANCE_FUNC = NULL;
Term* LAMBDA_FUNC = NULL;
Term* LENGTH_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_FUNC = NULL;
Term* LIST_APPEND_FUNC = NULL;
Term* LOAD_SCRIPT_FUNC = NULL;
Term* MULT_FUNC = NULL;
Term* NAMESPACE_FUNC = NULL;
Term* NEG_FUNC = NULL;
Term* NOT_FUNC = NULL;
Term* ONE_TIME_ASSIGN_FUNC = NULL;
Term* OUTPUT_PLACEHOLDER_FUNC = NULL;
Term* OVERLOADED_FUNCTION_FUNC = NULL;
Term* PACK_STATE_FUNC = NULL;
Term* PRESERVE_STATE_RESULT_FUNC = NULL;
Term* RANGE_FUNC = NULL;
Term* REF_FUNC = NULL;
Term* RETURN_FUNC = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SET_INDEX_FUNC = NULL;
Term* SUBROUTINE_OUTPUT_FUNC = NULL;
Term* SWITCH_FUNC = NULL;
Term* STATEFUL_VALUE_FUNC = NULL;
Term* STATIC_ERROR_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* TYPE_FUNC = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNPACK_STATE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;
Term* UNSAFE_ASSIGN_FUNC = NULL;
Term* VALUE_FUNC = NULL;

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* DICT_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* NULL_T_TERM = NULL;
Term* RECT_I_TYPE_TERM = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* COLOR_TYPE = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* SYMBOL_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* VOID_TYPE = NULL;
Term* OPAQUE_POINTER_TYPE;

// Builtin type objects:
Type ANY_T;
Type BOOL_T;
Type BRANCH_T;
Type DICT_T;
Type ERROR_T;
Type EVAL_CONTEXT_T;
Type FLOAT_T;
Type FUNCTION_T;
Type FUNCTION_ATTRS_T;
Type HANDLE_T;
Type INT_T;
Type LIST_T;
Type NULL_T;
Type OPAQUE_POINTER_T;
Type REF_T;
Type STRING_T;
Type SYMBOL_T;
Type TYPE_T;
Type VOID_T;

// Input instructions:
Type StackVariableIsn_t;
Type GlobalVariableIsn_t;
Type ImplicitStateInputIsn_t;

// Builtin symbols:
TaggedValue FileSymbol;
TaggedValue OutSymbol;
TaggedValue RepeatSymbol;
TaggedValue UnknownSymbol;

// Symbols for static errors
TaggedValue NotEnoughInputsSymbol;
TaggedValue TooManyInputsSymbol;
TaggedValue ExtraOutputNotFoundSymbol;

TaggedValue TrueValue;
TaggedValue FalseValue;

Type* FILE_SIGNATURE_T;

namespace cppbuild_function { CA_FUNCTION(build_module); }

// Standard library functions
CA_FUNCTION(evaluate_output_placeholder)
{
    copy(INPUT(0), OUTPUT);
}

CA_FUNCTION(file__modified_time)
{
    set_int(OUTPUT, get_modified_time(STRING_INPUT(0)));
}

CA_FUNCTION(input_func)
{
    int index = INT_INPUT(0);
    TaggedValue* input = CONTEXT->inputStack.getLast()->getIndex(index);
    if (input == NULL)
        return ERROR_OCCURRED("invalid input index");
    copy(input, OUTPUT);
}

CA_FUNCTION(refactor__rename)
{
    rename(as_ref(INPUT(0)), as_string(INPUT(1)));
}

CA_FUNCTION(refactor__change_function)
{
    change_function(as_ref(INPUT_TERM(0)), INPUT_TERM(1));
}

CA_FUNCTION(reflect__this_branch)
{
    set_branch(OUTPUT, CALLER->owningBranch);
}

CA_FUNCTION(length)
{
    set_int(OUTPUT, num_elements(INPUT(0)));
}

CA_FUNCTION(type_func)
{
    set_type(OUTPUT, declared_type(INPUT_TERM(0)));
}

CA_FUNCTION(typename_func)
{
    set_string(OUTPUT, declared_type(INPUT_TERM(0))->name);
}

std::string stackVariable_toString(TaggedValue* value)
{
    short relativeFrame = value->value_data.asint >> 16;
    short index = (value->value_data.asint & 0xffff);
    std::stringstream strm;
    strm << "[frame:" << relativeFrame << ", index:" << index << "]";
    return strm.str();
}

Branch* kernel()
{
    return KERNEL;
}

void create_primitive_types()
{
    null_t::setup_type(&NULL_T);
    bool_t::setup_type(&BOOL_T);
    branch_setup_type(&BRANCH_T);
    dict_t::setup_type(&DICT_T);
    eval_context_t::setup_type(&EVAL_CONTEXT_T);
    float_t::setup_type(&FLOAT_T);
    handle_t::setup_type(&HANDLE_T);
    int_t::setup_type(&INT_T);
    list_t::setup_type(&LIST_T);
    string_t::setup_type(&STRING_T);
    symbol_t::setup_type(&SYMBOL_T);
    ref_t::setup_type(&REF_T);
    void_t::setup_type(&VOID_T);
    opaque_pointer_t::setup_type(&OPAQUE_POINTER_T);
    eval_context_setup_type(&EVAL_CONTEXT_T);

    // errors are just stored as strings for now
    string_t::setup_type(&ERROR_T);

    symbol_t::assign_new_symbol("file", &FileSymbol);
    symbol_t::assign_new_symbol("repeat", &RepeatSymbol);
    symbol_t::assign_new_symbol("out", &OutSymbol);
    symbol_t::assign_new_symbol("unknown", &UnknownSymbol);
    symbol_t::assign_new_symbol("not_enough_inputs", &NotEnoughInputsSymbol);
    symbol_t::assign_new_symbol("too_many_inputs", &TooManyInputsSymbol);
    symbol_t::assign_new_symbol("extra_output_not_found", &ExtraOutputNotFoundSymbol);


    // input instructions
    StackVariableIsn_t.name = "StackVariableIsn";
    StackVariableIsn_t.storageType = STORAGE_TYPE_INT;
    StackVariableIsn_t.toString = stackVariable_toString;
    GlobalVariableIsn_t.name = "GlobalVariableIsn";
    GlobalVariableIsn_t.storageType = STORAGE_TYPE_REF;
    ImplicitStateInputIsn_t.name = "ImplicitStateInputIsn";
    ImplicitStateInputIsn_t.storageType = STORAGE_TYPE_INT;
}

void bootstrap_kernel()
{
    // Create the very first building blocks. These elements need to be in place
    // before we can parse code in the proper way.

    KERNEL = new Branch();

    Branch* kernel = KERNEL;

    // Create value function
    VALUE_FUNC = kernel->appendNew();
    kernel->bindName(VALUE_FUNC, "value");

    // Create Type type
    TYPE_TYPE = kernel->appendNew();
    TYPE_TYPE->function = VALUE_FUNC;
    TYPE_TYPE->type = &TYPE_T;
    TYPE_TYPE->value_type = &TYPE_T;
    TYPE_TYPE->value_data.ptr = &TYPE_T;
    type_t::setup_type(&TYPE_T);
    kernel->bindName(TYPE_TYPE, "Type");

    // Create Any type
    ANY_TYPE = kernel->appendNew();
    ANY_TYPE->function = VALUE_FUNC;
    ANY_TYPE->type = &TYPE_T;
    ANY_TYPE->value_type = &TYPE_T;
    ANY_TYPE->value_data.ptr = &ANY_T;
    any_t::setup_type(&ANY_T);
    kernel->bindName(ANY_TYPE, "any");

    // Create Function type
    function_t::setup_type(&FUNCTION_T);
    FUNCTION_TYPE = kernel->appendNew();
    FUNCTION_TYPE->function = VALUE_FUNC;
    FUNCTION_TYPE->type = &TYPE_T;
    FUNCTION_TYPE->value_type = &TYPE_T;
    FUNCTION_TYPE->value_data.ptr = &FUNCTION_T;
    kernel->bindName(FUNCTION_TYPE, "Function");

    // Initialize value() func
    VALUE_FUNC->type = &FUNCTION_T;
    VALUE_FUNC->function = VALUE_FUNC;
    create(&FUNCTION_T, (TaggedValue*)VALUE_FUNC);

    function_t::initialize(&FUNCTION_T, VALUE_FUNC);
    initialize_function(VALUE_FUNC);

    // Initialize primitive types (this requires value() function)
    BOOL_TYPE = create_type_value(kernel, &BOOL_T, "bool");
    FLOAT_TYPE = create_type_value(kernel, &FLOAT_T, "number");
    INT_TYPE = create_type_value(kernel, &INT_T, "int");
    NULL_T_TERM = create_type_value(kernel, &NULL_T, "Null");
    STRING_TYPE = create_type_value(kernel, &STRING_T, "string");
    SYMBOL_TYPE = create_type_value(kernel, &SYMBOL_T, "Symbol");
    DICT_TYPE = create_type_value(kernel, &DICT_T, "Dict");
    REF_TYPE = create_type_value(kernel, &REF_T, "Ref");
    VOID_TYPE = create_type_value(kernel, &VOID_T, "void");
    LIST_TYPE = create_type_value(kernel, &LIST_T, "List");
    OPAQUE_POINTER_TYPE = create_type_value(kernel, &OPAQUE_POINTER_T, "opaque_pointer");
    create_type_value(kernel, &BRANCH_T, "Branch");

    // Setup output_placeholder() function, needed to declare functions properly.
    OUTPUT_PLACEHOLDER_FUNC = create_value(kernel, &FUNCTION_T, "output_placeholder");
    function_t::initialize(&FUNCTION_T, OUTPUT_PLACEHOLDER_FUNC);
    initialize_function(OUTPUT_PLACEHOLDER_FUNC);
    as_function(OUTPUT_PLACEHOLDER_FUNC)->name = "output_placeholder";
    as_function(OUTPUT_PLACEHOLDER_FUNC)->evaluate = evaluate_output_placeholder;
    ca_assert(function_get_output_type(OUTPUT_PLACEHOLDER_FUNC, 0) == &ANY_T);

    // Fix some holes in value() function
    Function* attrs = as_function(VALUE_FUNC);
    finish_building_function(attrs, &ANY_T);

    ca_assert(function_get_output_type(VALUE_FUNC, 0) == &ANY_T);

    // input_placeholder() is needed before we can declare a function with inputs
    INPUT_PLACEHOLDER_FUNC = import_function(kernel, NULL, "input_placeholder() -> any");
    ADDITIONAL_OUTPUT_FUNC = import_function(kernel, NULL, "additional_output() -> any");

    // Now that we have input_placeholder() let's declare one input on output_placeholder()
    apply(function_contents(as_function(OUTPUT_PLACEHOLDER_FUNC)),
        INPUT_PLACEHOLDER_FUNC, TermList())->setBoolProp("optional", true);

    // FileSignature is used in some builtin functions
    FILE_SIGNATURE_T = unbox_type(parse_type(kernel,
            "type FileSignature { string filename, int time_modified }"));

    namespace_function::early_setup(kernel);

    // Set up some global constants
    set_bool(&TrueValue, true);
    set_bool(&FalseValue, false);
}

void initialize_compound_types(Branch* kernel)
{
    Term* set_type = parse_type(kernel, "type Set;");
    set_t::setup_type(unbox_type(set_type));

    Term* map_type = parse_type(kernel, "type Map;");
    hashtable_t::setup_type(unbox_type(map_type));

    Term* styledSourceType = parse_type(kernel, "type StyledSource;");
    styled_source_t::setup_type(unbox_type(styledSourceType));

    Term* indexableType = parse_type(kernel, "type Indexable;");
    indexable_t::setup_type(unbox_type(indexableType));

    callable_t::setup_type(unbox_type(parse_type(kernel, "type Callable;")));

    RECT_I_TYPE_TERM = parse_type(kernel, "type Rect_i { int x1, int y1, int x2, int y2 }");
}

void pre_setup_builtin_functions(Branch* kernel)
{
    return_function::setup(kernel);
}

void post_setup_functions(Branch* kernel)
{
    // Create vectorized add() functions
    Term* add_v = create_duplicate(kernel, kernel->get("vectorize_vv"), "add_v");
    set_ref(&as_function(add_v)->parameter, ADD_FUNC);
    overloaded_function::append_overload(ADD_FUNC, add_v);

    Term* add_s = create_duplicate(kernel, kernel->get("vectorize_vs"), "add_s");
    set_ref(&as_function(add_s)->parameter, ADD_FUNC);
    overloaded_function::append_overload(ADD_FUNC, add_s);

    // Create vectorized sub() functions
    Term* sub_v = create_duplicate(kernel, kernel->get("vectorize_vv"), "sub_v");
    set_ref(&as_function(sub_v)->parameter, SUB_FUNC);
    overloaded_function::append_overload(SUB_FUNC, sub_v);

    Term* sub_s = create_duplicate(kernel, kernel->get("vectorize_vs"), "sub_s");
    set_ref(&as_function(sub_s)->parameter, SUB_FUNC);
    overloaded_function::append_overload(SUB_FUNC, sub_s);

    // Create vectorized mult() functions
    Term* mult_v = create_duplicate(kernel, kernel->get("vectorize_vv"), "mult_v");
    set_ref(&as_function(mult_v)->parameter, kernel->get("mult"));
    overloaded_function::append_overload(MULT_FUNC, mult_v);

    Term* mult_s = create_duplicate(kernel, kernel->get("vectorize_vs"), "mult_s");
    set_ref(&as_function(mult_s)->parameter, kernel->get("mult"));
    overloaded_function::append_overload(MULT_FUNC, mult_s);

    // Create vectorized div() function
    Term* div_s = create_duplicate(kernel, kernel->get("vectorize_vs"), "div_s");
    set_ref(&as_function(div_s)->parameter, DIV_FUNC);
    overloaded_function::append_overload(DIV_FUNC, div_s);
}

void parse_hosted_types(Branch* kernel)
{
    parse_type(kernel, "type Point { number x, number y }");
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");

    COLOR_TYPE = parse_type(kernel, "type Color { number r, number g, number b, number a }");

    color_t::setup_type(unbox_type(COLOR_TYPE));
}

void install_standard_library(Branch* kernel)
{
    // Parse the stdlib script
    parser::compile(kernel, parser::statement_list, STDLIB_CA_TEXT);

    // Install each function
    install_function(kernel->get("file:modified_time"), file__modified_time);
    install_function(kernel->get("input"), input_func);
    install_function(kernel->get("length"), length);
    install_function(kernel->get("type"), type_func);
    install_function(kernel->get("typename"), typename_func);
    install_function(kernel->get("refactor:rename"), refactor__rename);
    install_function(kernel->get("refactor:change_function"), refactor__change_function);
    install_function(kernel->get("reflect:this_branch"), reflect__this_branch);

    install_function(kernel->get("cppbuild:build_module"), cppbuild_function::build_module);

    LENGTH_FUNC = kernel->get("length");
    TYPE_FUNC = kernel->get("type");
}

export_func void circa_initialize()
{
    FINISHED_BOOTSTRAP = false;
    STATIC_INITIALIZATION_FINISHED = true;

    create_primitive_types();
    bootstrap_kernel();

    Branch* kernel = KERNEL;

    initialize_compound_types(kernel);

    FINISHED_BOOTSTRAP = true;

    pre_setup_builtin_functions(kernel);
    setup_builtin_functions(kernel);
    post_setup_functions(kernel);
    parse_hosted_types(kernel);

    type_initialize_kernel(kernel);

    // Install C functions into stdlib
    install_standard_library(kernel);

#if CIRCA_TEST_BUILD
    // Create a space for unit tests.
    create_branch(kernel, "_test_root");
#endif

    // Finally, make sure there are no static errors.
    if (has_static_errors(kernel)) {
        std::cout << "Static errors found in kernel:" << std::endl;
        print_static_errors_formatted(kernel, std::cout);
        return;
    }
}

export_func void circa_shutdown()
{
    SHUTTING_DOWN = true;

    clear_type_contents(&BOOL_T);
    clear_type_contents(&DICT_T);
    clear_type_contents(&ERROR_T);
    clear_type_contents(&FLOAT_T);
    clear_type_contents(&INT_T);
    clear_type_contents(&LIST_T);
    clear_type_contents(&NULL_T);
    clear_type_contents(&OPAQUE_POINTER_T);
    clear_type_contents(&REF_T);
    clear_type_contents(&STRING_T);
    clear_type_contents(&TYPE_T);
    clear_type_contents(&VOID_T);

    gc_collect();

    delete KERNEL;
    KERNEL = NULL;
}

} // namespace circa
