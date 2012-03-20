// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"
#include "circa/file.h"

#include "branch.h"
#include "building.h"
#include "dict.h"
#include "evaluation.h"
#include "filesystem.h"
#include "function.h"
#include "gc.h"
#include "generic.h"
#include "importing.h"
#include "importing_macros.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "parser.h"
#include "subroutine.h"
#include "static_checking.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "type.h"

#include "types/any.h"
#include "types/bool.h"
#include "types/callable.h"
#include "types/color.h"
#include "types/common.h"
#include "types/eval_context.h"
#include "types/hashtable.h"
#include "types/indexable.h"
#include "types/int.h"
#include "types/name.h"
#include "types/number.h"
#include "types/ref.h"
#include "types/set.h"
#include "types/void.h"

#include "tools/command_reader.h"

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

Term* APPLY_FEEDBACK = NULL;
Term* AVERAGE_FUNC = NULL;
Term* BRANCH_UNEVALUATED_FUNC = NULL;
Term* DESIRED_VALUE_FEEDBACK = NULL;
Term* DIV_FUNC = NULL;
Term* DO_ONCE_FUNC = NULL;
Term* ERRORED_FUNC = NULL;
Term* EXTRA_OUTPUT_FUNC = NULL;
Term* FEEDBACK_FUNC = NULL;
Term* FREEZE_FUNC = NULL;
Term* FOR_FUNC = NULL;
Term* GET_INDEX_FROM_BRANCH_FUNC = NULL;
Term* IF_BLOCK_FUNC = NULL;
Term* COND_FUNC = NULL;
Term* INCLUDE_FUNC = NULL;
Term* INSTANCE_FUNC = NULL;
Term* LAMBDA_FUNC = NULL;
Term* LENGTH_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_APPEND_FUNC = NULL;
Term* LOAD_SCRIPT_FUNC = NULL;
Term* MULT_FUNC = NULL;
Term* NAMESPACE_FUNC = NULL;
Term* NEG_FUNC = NULL;
Term* NOT_FUNC = NULL;
Term* OVERLOADED_FUNCTION_FUNC = NULL;
Term* RANGE_FUNC = NULL;
Term* REF_FUNC = NULL;
Term* RETURN_FUNC = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SET_INDEX_FUNC = NULL;
Term* SWITCH_FUNC = NULL;
Term* STATEFUL_VALUE_FUNC = NULL;
Term* STATIC_ERROR_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* TYPE_FUNC = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* DICT_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* NULL_T_TERM = NULL;
Term* RECT_I_TYPE_TERM = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* NAME_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* VOID_TYPE = NULL;
Term* OPAQUE_POINTER_TYPE = NULL;

// New style for builtin function pointers
BuiltinFuncs FUNCS;

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
Type NAME_T;
Type TYPE_T;
Type VOID_T;

BuiltinTypes TYPES;

caValue TrueValue;
caValue FalseValue;

namespace cppbuild_function { CA_FUNCTION(build_module); }

// Standard library functions
CA_FUNCTION(evaluate_output_placeholder)
{
    CONSUME_INPUT(0, OUTPUT);
}

Type* output_placeholder_specializeType(Term* caller)
{
    if (caller->input(0) == NULL)
        return NULL;
    return declared_type(caller->input(0));
}

CA_FUNCTION(file__exists)
{
    set_bool(OUTPUT, circa_file_exists(STRING_INPUT(0)));
}
CA_FUNCTION(file__version)
{
    set_int(OUTPUT, circa_file_get_version(STRING_INPUT(0)));
}
CA_FUNCTION(file__read_text)
{
    set_string(OUTPUT, circa_read_file(STRING_INPUT(0)));
}

CA_FUNCTION(file__fetch_record)
{
    const char* filename = STRING_INPUT(0);
    Name name = INT_INPUT(1);
    circa_create_default_output((caStack*) CONTEXT, 0);
    set_pointer(OUTPUT, circa_fetch_file_record(filename, name));
}

CA_FUNCTION(from_string)
{
    circa_parse_string(STRING_INPUT(0), (caValue*) OUTPUT);
}

CA_FUNCTION(to_string_repr)
{
    circa_to_string_repr(INPUT(0), OUTPUT);
}

CA_FUNCTION(input_func)
{
    int index = INT_INPUT(0);
    caValue* input = CONTEXT->argumentList.getLast()->getIndex(index);
    if (input == NULL)
        return RAISE_ERROR("invalid input index");
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

CA_FUNCTION(reflect__kernel)
{
    set_branch(OUTPUT, kernel());
}

CA_FUNCTION(sys__module_search_paths)
{
    copy(modules_get_search_paths(), OUTPUT);
}

CA_FUNCTION(sys__do_admin_command)
{
    do_admin_command(INPUT(0), OUTPUT);
}

CA_FUNCTION(Branch__dump)
{
    dump(as_branch(INPUT(0)));
}

CA_FUNCTION(Function__name)
{
    set_string(OUTPUT, as_function(INPUT(0))->name);
}

CA_FUNCTION(length)
{
    set_int(OUTPUT, num_elements(INPUT(0)));
}

std::string stackVariable_toString(caValue* value)
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
    number_t::setup_type(&FLOAT_T);
    // handle_t::setup_type(&HANDLE_T);
    int_t::setup_type(&INT_T);
    list_t::setup_type(&LIST_T);
    name_t::setup_type(&NAME_T);
    opaque_pointer_t::setup_type(&OPAQUE_POINTER_T);
    ref_t::setup_type(&REF_T);
    string_setup_type(&STRING_T);
    void_t::setup_type(&VOID_T);
    eval_context_setup_type(&EVAL_CONTEXT_T);

    // errors are just stored as strings for now
    string_setup_type(&ERROR_T);
}

void bootstrap_kernel()
{
    // Create the very first building blocks. These elements need to be in place
    // before we can parse code in the proper way.

    KERNEL = new Branch();
    Branch* kernel = KERNEL;

    // Create value function
    Term* valueFunc = kernel->appendNew();
    rename(valueFunc, "value");
    FUNCS.value = valueFunc;

    // Create Type type
    TYPE_TYPE = kernel->appendNew();
    TYPE_TYPE->function = FUNCS.value;
    TYPE_TYPE->type = &TYPE_T;
    TYPE_TYPE->value_type = &TYPE_T;
    TYPE_TYPE->value_data.ptr = &TYPE_T;
    type_t::setup_type(&TYPE_T);
    rename(TYPE_TYPE, "Type");

    // Create Any type
    ANY_TYPE = kernel->appendNew();
    ANY_TYPE->function = valueFunc;
    ANY_TYPE->type = &TYPE_T;
    ANY_TYPE->value_type = &TYPE_T;
    ANY_TYPE->value_data.ptr = &ANY_T;
    any_t::setup_type(&ANY_T);
    rename(ANY_TYPE, "any");

    // Create Function type
    function_t::setup_type(&FUNCTION_T);
    FUNCTION_TYPE = kernel->appendNew();
    FUNCTION_TYPE->function = valueFunc;
    FUNCTION_TYPE->type = &TYPE_T;
    FUNCTION_TYPE->value_type = &TYPE_T;
    FUNCTION_TYPE->value_data.ptr = &FUNCTION_T;
    rename(FUNCTION_TYPE, "Function");

    // Initialize value() func
    valueFunc->type = &FUNCTION_T;
    valueFunc->function = valueFunc;
    create(&FUNCTION_T, (caValue*)valueFunc);

    function_t::initialize(&FUNCTION_T, valueFunc);
    initialize_function(valueFunc);
    as_function(valueFunc)->name = "value";

    // Initialize primitive types (this requires value() function)
    BOOL_TYPE = create_type_value(kernel, &BOOL_T, "bool");
    FLOAT_TYPE = create_type_value(kernel, &FLOAT_T, "number");
    INT_TYPE = create_type_value(kernel, &INT_T, "int");
    NAME_TYPE = create_type_value(kernel, &NAME_T, "Name");
    NULL_T_TERM = create_type_value(kernel, &NULL_T, "Null");
    STRING_TYPE = create_type_value(kernel, &STRING_T, "string");
    DICT_TYPE = create_type_value(kernel, &DICT_T, "Dict");
    REF_TYPE = create_type_value(kernel, &REF_T, "Term");
    VOID_TYPE = create_type_value(kernel, &VOID_T, "void");
    LIST_TYPE = create_type_value(kernel, &LIST_T, "List");
    OPAQUE_POINTER_TYPE = create_type_value(kernel, &OPAQUE_POINTER_T, "opaque_pointer");
    create_type_value(kernel, &BRANCH_T, "Branch");

    // Setup output_placeholder() function, needed to declare functions properly.
    FUNCS.output = create_value(kernel, &FUNCTION_T, "output_placeholder");
    function_t::initialize(&FUNCTION_T, FUNCS.output);
    initialize_function(FUNCS.output);
    as_function(FUNCS.output)->name = "output_placeholder";
    as_function(FUNCS.output)->evaluate = evaluate_output_placeholder;
    as_function(FUNCS.output)->specializeType = output_placeholder_specializeType;
    ca_assert(function_get_output_type(FUNCS.output, 0) == &ANY_T);

    // Fix some holes in value() function
    Function* attrs = as_function(valueFunc);
    finish_building_function(attrs, &ANY_T);

    ca_assert(function_get_output_type(valueFunc, 0) == &ANY_T);

    // input_placeholder() is needed before we can declare a function with inputs
    FUNCS.input = import_function(kernel, NULL, "input_placeholder() -> any");

    // Now that we have input_placeholder() let's declare one input on output_placeholder()
    apply(function_contents(as_function(FUNCS.output)),
        FUNCS.input, TermList())->setBoolProp("optional", true);

    // FileSignature is used in some builtin functions
    TYPES.file_signature = unbox_type(parse_type(kernel,
            "type FileSignature { string filename, int time_modified }"));

    namespace_function::early_setup(kernel);

    // Set up some global constants
    set_bool(&TrueValue, true);
    set_bool(&FalseValue, false);

    FINISHED_BOOTSTRAP = true;

    // Initialize compound types
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

    // Setup all the builtin functions defined in src/functions
    setup_builtin_functions(kernel);

    // Create IMPLICIT_TYPES (deprecated)
    type_initialize_kernel(kernel);

    // Now we can build derived functions

    // Create overloaded functions
    FUNCS.add = create_overloaded_function(kernel, "add(any,any) -> any");
    append_to_overloaded_function(FUNCS.add, FUNCS.add_i);
    append_to_overloaded_function(FUNCS.add, FUNCS.add_f);

    Term* less_than = create_overloaded_function(kernel, "less_than(any,any) -> bool");
    append_to_overloaded_function(less_than, kernel->get("less_than_i"));
    append_to_overloaded_function(less_than, kernel->get("less_than_f"));

    Term* less_than_eq = create_overloaded_function(kernel, "less_than_eq(any,any) -> bool");
    append_to_overloaded_function(less_than_eq, kernel->get("less_than_eq_i"));
    append_to_overloaded_function(less_than_eq, kernel->get("less_than_eq_f"));

    Term* greater_than = create_overloaded_function(kernel, "greater_than(any,any) -> bool");
    append_to_overloaded_function(greater_than, kernel->get("greater_than_i"));
    append_to_overloaded_function(greater_than, kernel->get("greater_than_f"));

    Term* greater_than_eq = create_overloaded_function(kernel, "greater_than_eq(any,any) -> bool");
    append_to_overloaded_function(greater_than_eq, kernel->get("greater_than_eq_i"));
    append_to_overloaded_function(greater_than_eq, kernel->get("greater_than_eq_f"));

    Term* max_func = create_overloaded_function(kernel, "max(any,any) -> any");
    append_to_overloaded_function(max_func, kernel->get("max_i"));
    append_to_overloaded_function(max_func, kernel->get("max_f"));

    Term* min_func = create_overloaded_function(kernel, "min(any,any) -> any");
    append_to_overloaded_function(min_func, kernel->get("min_i"));
    append_to_overloaded_function(min_func, kernel->get("min_f"));

    Term* remainder_func = create_overloaded_function(kernel, "remainder(any,any) -> any");
    append_to_overloaded_function(remainder_func, kernel->get("remainder_i"));
    append_to_overloaded_function(remainder_func, kernel->get("remainder_f"));

    Term* mod_func = create_overloaded_function(kernel, "max(any,any) -> any");
    append_to_overloaded_function(mod_func, kernel->get("max_i"));
    append_to_overloaded_function(mod_func, kernel->get("max_f"));

    MULT_FUNC = create_overloaded_function(kernel, "mult(any,any) -> any");
    //dump(nested_contents(MULT_FUNC));
    append_to_overloaded_function(MULT_FUNC, kernel->get("mult_i"));
    append_to_overloaded_function(MULT_FUNC, kernel->get("mult_f"));

    NEG_FUNC = create_overloaded_function(kernel, "neg(any) -> any");
    append_to_overloaded_function(NEG_FUNC, kernel->get("neg_i"));
    append_to_overloaded_function(NEG_FUNC, kernel->get("neg_f"));
    as_function(NEG_FUNC)->formatSource = neg_function::formatSource;

    SUB_FUNC = create_overloaded_function(kernel, "sub(any,any) -> any");
    append_to_overloaded_function(SUB_FUNC, kernel->get("sub_i"));
    append_to_overloaded_function(SUB_FUNC, kernel->get("sub_f"));

    // Create vectorized functions
    Term* add_v = create_function(kernel, "add_v");
    create_function_vectorized_vv(function_contents(add_v), FUNCS.add, &LIST_T, &LIST_T);
    Term* add_s = create_function(kernel, "add_s");
    create_function_vectorized_vs(function_contents(add_s), FUNCS.add, &LIST_T, &ANY_T);

    append_to_overloaded_function(FUNCS.add, add_v);
    append_to_overloaded_function(FUNCS.add, add_s);

    //dump(function_contents(add_v));
    //dump(function_contents(FUNCS.add));

    Term* sub_v = create_function(kernel, "sub_v");
    create_function_vectorized_vv(function_contents(sub_v), SUB_FUNC, &LIST_T, &LIST_T);
    Term* sub_s = create_function(kernel, "sub_s");
    create_function_vectorized_vs(function_contents(sub_s), SUB_FUNC, &LIST_T, &ANY_T);
    
    append_to_overloaded_function(SUB_FUNC, sub_v);
    append_to_overloaded_function(SUB_FUNC, sub_s);

    // Create vectorized mult() functions
    Term* mult_v = create_function(kernel, "mult_v");
    create_function_vectorized_vv(function_contents(mult_v), MULT_FUNC, &LIST_T, &LIST_T);
    Term* mult_s = create_function(kernel, "mult_s");
    create_function_vectorized_vs(function_contents(mult_s), MULT_FUNC, &LIST_T, &ANY_T);

    append_to_overloaded_function(MULT_FUNC, mult_v);
    append_to_overloaded_function(MULT_FUNC, mult_s);

    Term* div_s = create_function(kernel, "div_s");
    create_function_vectorized_vs(function_contents(div_s), DIV_FUNC, &LIST_T, &ANY_T);

    // Create some hosted types
    TYPES.point = as_type(parse_type(kernel, "type Point { number x, number y }"));
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");

    TYPES.color = unbox_type(parse_type(kernel,
                    "type Color { number r, number g, number b, number a }"));

    color_t::setup_type(TYPES.color);
}

void install_standard_library(Branch* kernel)
{
    // Parse the stdlib script
    parser::compile(kernel, parser::statement_list, STDLIB_CA_TEXT);

    // Install each function
    install_function(kernel->get("cppbuild:build_module"), cppbuild_function::build_module);
    install_function(kernel->get("file:version"), file__version);
    install_function(kernel->get("file:exists"), file__exists);
    install_function(kernel->get("file:read_text"), file__read_text);
    install_function(kernel->get("file:fetch_record"), file__fetch_record);
    install_function(kernel->get("input"), input_func);
    install_function(kernel->get("length"), length);
    install_function(kernel->get("from_string"), from_string);
    install_function(kernel->get("to_string_repr"), to_string_repr);
    install_function(kernel->get("refactor:rename"), refactor__rename);
    install_function(kernel->get("refactor:change_function"), refactor__change_function);
    install_function(kernel->get("reflect:this_branch"), reflect__this_branch);
    install_function(kernel->get("reflect:kernel"), reflect__kernel);
    install_function(kernel->get("sys:module_search_paths"), sys__module_search_paths);
    install_function(kernel->get("sys:do_admin_command"), sys__do_admin_command);
    install_function(kernel->get("Branch.dump"), Branch__dump);
    install_function(kernel->get("Function.name"), Function__name);

    LENGTH_FUNC = kernel->get("length");
    TYPE_FUNC = kernel->get("type");

    FUNCS.dll_patch = kernel->get("sys:dll_patch");
}

EXPORT void circa_initialize()
{
    FINISHED_BOOTSTRAP = false;
    STATIC_INITIALIZATION_FINISHED = true;

    memset(&FUNCS, 0, sizeof(FUNCS));
    memset(&TYPES, 0, sizeof(TYPES));

    create_primitive_types();
    bootstrap_kernel();

    Branch* kernel = KERNEL;

    install_standard_library(kernel);

    // Make sure there are no static errors. This shouldn't happen.
    if (has_static_errors(kernel)) {
        std::cout << "Static errors found in kernel:" << std::endl;
        print_static_errors_formatted(kernel, std::cout);
        internal_error("circa fatal: static errors found in kernel");
    }

#if CIRCA_ENABLE_FILESYSTEM
    // Use standard filesystem by default
    circa_use_standard_filesystem();
#endif

    // Load library paths from CIRCA_LIB_PATH
    const char* libPathEnv = getenv("CIRCA_LIB_PATH");
    if (libPathEnv != NULL) {
        caValue libPathStr;
        set_string(&libPathStr, libPathEnv);

        caValue libPaths;
        string_split(&libPathStr, ';', &libPaths);

        for (int i=0; i < list_length(&libPaths); i++) {
            caValue* path = list_get(&libPaths, i);
            if (string_eq(path, ""))
                continue;
            modules_add_search_path(as_cstring(path));
        }
    }
}

EXPORT void circa_shutdown()
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

    delete KERNEL;
    KERNEL = NULL;

    memset(&FUNCS, 0, sizeof(FUNCS));

    name_dealloc_global_data();

    gc_collect();
}

} // namespace circa
