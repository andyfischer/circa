// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"
#include "circa/file.h"

#include "blob.h"
#include "block.h"
#include "building.h"
#include "closures.h"
#include "control_flow.h"
#include "code_iterators.h"
#include "function.h"
#include "generic.h"
#include "hashtable.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "misc_builtins.h"
#include "native_patch.h"
#include "parser.h"
#include "reflection.h"
#include "selector.h"
#include "source_repro.h"
#include "stack.h"
#include "stateful_code.h"
#include "static_checking.h"
#include "string_repr.h"
#include "string_type.h"
#include "symbols.h"
#include "names.h"
#include "term.h"
#include "type_inference.h"
#include "type.h"
#include "world.h"

#include "types/any.h"
#include "types/bool.h"
#include "types/color.h"
#include "types/common.h"
#include "types/int.h"
#include "types/number.h"
#include "types/set.h"
#include "types/void.h"

namespace circa {

World* g_world = NULL;

// STDLIB_CA_TEXT is defined in generated/stdlib_script_text.cpp
extern "C" {
    extern const char* STDLIB_CA_TEXT;
}

// setup_functions is defined in generated/setup_builtin_functions.cpp
void setup_builtin_functions(Block*);

BuiltinFuncs FUNCS;
BuiltinTypes TYPES;

caValue* g_oracleValues;
caValue* g_spyValues;

Type* output_placeholder_specializeType(Term* caller)
{
    // Special case: if we're an accumulatingOutput then the output type is List.
    if (caller->boolProp(sym_AccumulatingOutput, false))
        return TYPES.list;

    if (caller->input(0) == NULL)
        return NULL;

    return declared_type(caller->input(0));
}


caValue* find_context_value(caStack* stack, caValue* key)
{
    Frame* frame = stack_top(stack);

    while (frame != NULL) {
        if (!is_null(&frame->dynamicScope)) {
            caValue* value = hashtable_get(&frame->dynamicScope, key);
            if (value != NULL)
                return value;
        }

        frame = frame_parent(frame);
    }

    if (!is_null(&stack->topContext)) {
        caValue* value = hashtable_get(&stack->topContext, key);
        if (value != NULL)
            return value;
    }

    return NULL;
}

void get_context(caStack* stack)
{
    caValue* value = find_context_value(stack, circa_input(stack, 0));
    if (value != NULL)
        copy(value, circa_output(stack, 0));
    else
        set_null(circa_output(stack, 0));
}

void get_context_opt(caStack* stack)
{
    caValue* value = find_context_value(stack, circa_input(stack, 0));
    if (value != NULL)
        copy(value, circa_output(stack, 0));
    else
        copy(circa_input(stack, 1), circa_output(stack, 0));
}

void set_context(caStack* stack)
{
    caValue* key = circa_input(stack, 0);
    caValue* value = circa_input(stack, 1);

    Frame* frame = stack_top_parent(stack);

    if (!is_hashtable(&frame->dynamicScope))
        set_hashtable(&frame->dynamicScope);

    copy(value, hashtable_insert(&frame->dynamicScope, key));
}

void file__exists(caStack* stack)
{
    set_bool(circa_output(stack, 0),
        circa_file_exists(stack->world, circa_string_input(stack, 0)));
}
void file__version(caStack* stack)
{
    set_int(circa_output(stack, 0),
        circa_file_get_version(stack->world, circa_string_input(stack, 0)));
}
void file__read_text(caStack* stack)
{
    circa_read_file(stack->world, circa_string_input(stack, 0), circa_output(stack, 0));
}

void from_string(caStack* stack)
{
    parse_string_repr(circa_string_input(stack, 0), circa_output(stack, 0));
}

void to_string_repr(caStack* stack)
{
    write_string_repr(circa_input(stack, 0), circa_output(stack, 0));
}

void refactor__rename(caStack* stack)
{
    rename(as_term_ref(circa_input(stack, 0)), circa_input(stack, 1));
}

void refactor__change_function(caStack* stack)
{
    change_function(as_term_ref(circa_input(stack, 0)),
        (Term*) circa_caller_input_term(stack, 1));
}

void reflect__this_block(caStack* stack)
{
    set_block(circa_output(stack, 0), (Block*) circa_caller_block(stack));
}

void reflect__kernel(caStack* stack)
{
    set_block(circa_output(stack, 0), global_root_block());
}

void sys__module_search_paths(caStack* stack)
{
    copy(module_search_paths(stack->world), circa_output(stack, 0));
}
void sys__perf_stats_reset(caStack* stack)
{
    perf_stats_reset();
}
void sys__perf_stats_dump(caStack* stack)
{
    perf_stats_to_list(circa_output(stack, 0));
}
void global_script_version(caStack* stack)
{
    World* world = stack->world;
    set_int(circa_output(stack, 0), world->globalScriptVersion);
}

std::string stackVariable_toString(caValue* value)
{
    short relativeFrame = value->value_data.asint >> 16;
    short index = (value->value_data.asint & 0xffff);
    std::stringstream strm;
    strm << "[frame:" << relativeFrame << ", index:" << index << "]";
    return strm.str();
}

Type* specializeType_add_sub_mult(Term* term)
{
    Type* left = declared_type(term->input(0));
    Type* right = declared_type(term->input(1));

    if (left == TYPES.int_type && right == TYPES.int_type)
        return TYPES.int_type;

    if ((left == TYPES.int_type || left == TYPES.float_type)
            && (right == TYPES.int_type || right == TYPES.float_type))
        return TYPES.float_type;

    return TYPES.any;
}

Type* specializeType_div(Term* term)
{
    Type* left = declared_type(term->input(0));
    Type* right = declared_type(term->input(1));

    if ((left == TYPES.int_type || left == TYPES.float_type)
            && (right == TYPES.int_type || right == TYPES.float_type))
        return TYPES.float_type;

    return TYPES.any;
}

Type* List__append_specializeType(Term* term)
{
    Term* listInput = term->input(0);
    switch (list_get_parameter_type(&listInput->type->parameter)) {
    case sym_Untyped:
        return listInput->type;
    case sym_UniformListType:
    {
        Type* listElementType = list_get_repeated_type_from_type(listInput->type);
        Type* commonType = find_common_type(listElementType, term->input(1)->type);
        if (commonType == listElementType)
            return listInput->type;
        else
            return create_typed_unsized_list_type(commonType);
    }
    case sym_AnonStructType:
    case sym_StructType:
    {    
        List elementTypes;
        copy(list_get_type_list_from_type(listInput->type), &elementTypes);
        set_type(elementTypes.append(), term->input(1)->type);
        return create_typed_unsized_list_type(find_common_type(&elementTypes));
    }
    case sym_Invalid:
    default:
        return TYPES.any;
    }
}

void Mutable_initialize(Type* type, caValue* value)
{
    object_initialize(type, value);
    caValue* val = (caValue*) object_get_body(value);
    initialize_null(val);
}

std::string Mutable_toString(caValue* value)
{
    return "Mutable[" + to_string((caValue*) object_get_body(value)) + "]";
}

void Mutable_release(void* object)
{
    caValue* val = (caValue*) object;
    set_null(val);
}


World* global_world()
{
    return g_world;
}

Block* global_root_block()
{
    return global_world()->root;
}

Block* global_builtins_block()
{
    return global_world()->builtins;
}

std::string term_toString(caValue* val)
{
    Term* t = as_term_ref(val);
    if (t == NULL)
        return "Term#null";
    else {
        std::stringstream s;
        s << "Term#";
        s << t->id;
        return s.str();
    }
}

int term_hashFunc(caValue* val)
{
    Term* term = as_term_ref(val);
    if (term == NULL)
        return 0;
    return term->id;
}

void term_setup_type(Type* type)
{
    set_string(&type->name, "Term");
    type->storageType = sym_StorageTypeTerm;
    type->toString = term_toString;
    type->hashFunc = term_hashFunc;
}

// Spy & oracle
void test_spy_clear()
{
    if (g_spyValues == NULL)
        g_spyValues = circa_alloc_value();
    set_list(g_spyValues, 0);
}
caValue* test_spy_get_results()
{
    return g_spyValues;
}
void test_oracle_clear()
{
    if (g_oracleValues == NULL)
        g_oracleValues = circa_alloc_value();
    set_list(g_oracleValues, 0);
}
void test_oracle_send(caValue* value)
{
    copy(value, list_append(g_oracleValues));
}
caValue* test_oracle_append()
{
    return list_append(g_oracleValues);
}
void test_oracle_send(int i)
{
    set_int(list_append(g_oracleValues), i);
}
void test_oracle(caStack* stack)
{
    if (list_length(g_oracleValues) == 0)
        set_null(circa_output(stack, 0));
    else {
        copy(list_get(g_oracleValues, 0), circa_output(stack, 0));
        list_remove_index(g_oracleValues, 0);
    }
}

void test_spy(caStack* stack)
{
    if (g_spyValues == NULL) {
        g_spyValues = circa_alloc_value();
        set_list(g_spyValues);
    }
    copy(circa_input(stack, 0), list_append(g_spyValues));
}

void section_block_formatSource(caValue* source, Term* term)
{
    format_name_binding(source, term);
    append_phrase(source, "section", term, sym_None);
    append_phrase(source, " ", term, sym_Whitespace);
    format_block_source(source, nested_contents(term), term);
}

void nonlocal_formatSource(caValue* source, Term* term)
{
    append_phrase(source, as_cstring(term_name(term->input(0))), term, sym_None);
}

void for_each_root_type(void (*callback)(Type* type))
{
    (*callback)(TYPES.any);
    (*callback)(TYPES.blob);
    (*callback)(TYPES.block);
    (*callback)(TYPES.bool_type);
    (*callback)(TYPES.error);
    (*callback)(TYPES.float_type);
    (*callback)(TYPES.int_type);
    (*callback)(TYPES.list);
    (*callback)(TYPES.map);
    (*callback)(TYPES.opaque_pointer);
    (*callback)(TYPES.null);
    (*callback)(TYPES.string);
    (*callback)(TYPES.symbol);
    (*callback)(TYPES.term);
    (*callback)(TYPES.type);
    (*callback)(TYPES.void_type);
}

void bootstrap_kernel()
{
    memset(&FUNCS, 0, sizeof(FUNCS));
    memset(&TYPES, 0, sizeof(TYPES));

    // Allocate a World object.
    g_world = alloc_world();
    g_world->bootstrapStatus = sym_Bootstrapping;
    World* world = g_world;

    // Instanciate the types that are used by Type.
    TYPES.map = create_type_unconstructed();
    TYPES.null = create_type_unconstructed();
    TYPES.string = create_type_unconstructed();
    TYPES.type = create_type_unconstructed();

    // Now we can fully instanciate types.
    type_finish_construction(TYPES.map);
    type_finish_construction(TYPES.null);
    type_finish_construction(TYPES.string);
    type_finish_construction(TYPES.type);
    string_setup_type(TYPES.string);

    // Initialize remaining global types.
    TYPES.any = create_type();
    TYPES.blob = create_type();
    TYPES.block = create_type();
    TYPES.bool_type = create_type();
    TYPES.error = create_type();
    TYPES.float_type = create_type();
    TYPES.int_type = create_type();
    TYPES.list = create_type();
    TYPES.map = create_type();
    TYPES.opaque_pointer = create_type();
    TYPES.symbol = create_type();
    TYPES.term = create_type();
    TYPES.void_type = create_type();

    for_each_root_type(type_set_root);

    any_t::setup_type(TYPES.any);
    blob_setup_type(TYPES.blob);
    block_setup_type(TYPES.block);
    bool_t::setup_type(TYPES.bool_type);
    hashtable_setup_type(TYPES.map);
    int_t::setup_type(TYPES.int_type);
    list_t::setup_type(TYPES.list);
    symbol_setup_type(TYPES.symbol);
    null_t::setup_type(TYPES.null);
    number_t::setup_type(TYPES.float_type);
    opaque_pointer_t::setup_type(TYPES.opaque_pointer);
    term_setup_type(TYPES.term);
    string_setup_type(TYPES.error); // errors are just stored as strings for now
    type_t::setup_type(TYPES.type);
    void_t::setup_type(TYPES.void_type);

    // Create root Block.
    g_world->root = new Block();

    // Create builtins block.
    Term* builtinsTerm = g_world->root->appendNew();
    rename(builtinsTerm, "builtins");
    Block* builtins = make_nested_contents(builtinsTerm);
    g_world->builtins = builtins;

    // Create function_decl function.
    Term* functionDeclFunction = builtins->appendNew();
    rename(functionDeclFunction, "function_decl");
    FUNCS.function_decl = functionDeclFunction;
    FUNCS.function_decl->function = FUNCS.function_decl;
    make_nested_contents(FUNCS.function_decl);
    function_contents(FUNCS.function_decl)->overrides.formatSource = function_format_source;
    block_set_function_has_nested(function_contents(FUNCS.function_decl), true);

    // Create value function
    Term* valueFunc = builtins->appendNew();
    rename(valueFunc, "value");
    FUNCS.value = valueFunc;

    // Create Type type
    Term* typeType = builtins->appendNew();
    typeType->function = FUNCS.value;
    typeType->type = TYPES.type;
    term_value(typeType)->value_type = TYPES.type;
    term_value(typeType)->value_data.ptr = TYPES.type;
    TYPES.type->declaringTerm = typeType;
    rename(typeType, "Type");

    // Create Any type
    Term* anyType = builtins->appendNew();
    anyType->function = valueFunc;
    anyType->type = TYPES.type;
    term_value(anyType)->value_type = TYPES.type;
    term_value(anyType)->value_data.ptr = TYPES.any;
    TYPES.any->declaringTerm = anyType;
    rename(anyType, "any");

    // Initialize value() func
    valueFunc->type = TYPES.any;
    valueFunc->function = FUNCS.function_decl;
    make_nested_contents(valueFunc);
    block_set_evaluation_empty(function_contents(valueFunc), true);

    // Initialize primitive types (this requires value() function)
    create_type_value(builtins, TYPES.bool_type, "bool");
    create_type_value(builtins, TYPES.blob, "blob");
    create_type_value(builtins, TYPES.block, "Block");
    create_type_value(builtins, TYPES.float_type, "number");
    create_type_value(builtins, TYPES.int_type, "int");
    create_type_value(builtins, TYPES.list, "List");
    create_type_value(builtins, TYPES.opaque_pointer, "opaque_pointer");
    create_type_value(builtins, TYPES.string, "String");
    create_type_value(builtins, TYPES.symbol, "Symbol");
    create_type_value(builtins, TYPES.term, "Term");
    create_type_value(builtins, TYPES.void_type, "void");
    create_type_value(builtins, TYPES.map, "Map");

    // Finish initializing World (this requires List and Hashtable types)
    world_initialize(g_world);

    // Create global symbol table (requires Hashtable type)
    symbol_initialize_global_table();

    // Setup output_placeholder() function, needed to declare functions properly.
    FUNCS.output = apply(builtins, FUNCS.function_decl, TermList(), "output_placeholder");
    function_contents(FUNCS.output)->overrides.evaluate = NULL;
    function_contents(FUNCS.output)->overrides.specializeType = output_placeholder_specializeType;
    ca_assert(get_output_type(function_contents(FUNCS.output), 0) == TYPES.any);

    // Now that output_placeholder is created, fix the value() function.
    {
        Term* output = append_output_placeholder(function_contents(valueFunc), NULL);
        change_declared_type(output, TYPES.any);
        finish_building_function(function_contents(valueFunc));
    }

    ca_assert(get_output_type(function_contents(valueFunc), 0) == TYPES.any);

    // input_placeholder() is needed before we can declare a function with inputs
    FUNCS.input = import_function(builtins, NULL, "input_placeholder() -> any");
    block_set_evaluation_empty(function_contents(FUNCS.input), true);

    // Now that we have input_placeholder() let's declare one input on output_placeholder()
    apply(function_contents(FUNCS.output),
        FUNCS.input, TermList())->setBoolProp(sym_Optional, true);

    // Setup declare_field() function, needed to represent compound types.
    FUNCS.declare_field = import_function(builtins, NULL, "declare_field() -> any");

    // Initialize a few more types
    TYPES.selector = unbox_type(create_value(builtins, TYPES.type, "Selector"));
    list_t::setup_type(TYPES.selector);

    control_flow_setup_funcs(builtins);
    selector_setup_funcs(builtins);
    loop_setup_functions(builtins);

    // Setup all the builtin functions defined in src/functions
    setup_builtin_functions(builtins);

    FUNCS.section_block = import_function(builtins, NULL, "def section() -> any");
    block_set_format_source_func(function_contents(FUNCS.section_block), section_block_formatSource);

    FUNCS.case_condition_bool = import_function(builtins, NULL, "def case_condition_bool(bool condition)");
    FUNCS.loop_condition_bool = import_function(builtins, NULL, "def loop_condition_bool(bool condition)");
    FUNCS.minor_return_if_empty = import_function(builtins, NULL, "def minor_return_if_empty()");
    FUNCS.looped_input = import_function(builtins, NULL, "def looped_input(first, next) -> any");
    block_set_evaluation_empty(function_contents(FUNCS.looped_input), true);

    // dynamic_method() is needed before stdlib.ca.
    FUNCS.dynamic_method = import_function(builtins, NULL,
            "def dynamic_method(any inputs :multiple) -> any");

    FUNCS.func_call_implicit = import_function(builtins, NULL,
            "def func_call_implicit(any inputs :multiple) -> any");

    // Now we can build derived functions
    FUNCS.less_than = create_overloaded_function(builtins, "less_than(any a,any b) -> bool");
    append_to_overloaded_function(FUNCS.less_than, builtins->get("less_than_i"));
    append_to_overloaded_function(FUNCS.less_than, builtins->get("less_than_f"));
    finish_building_overloaded_function(FUNCS.less_than);

    FUNCS.less_than_eq = create_overloaded_function(builtins, "less_than_eq(any a,any b) -> bool");
    append_to_overloaded_function(FUNCS.less_than_eq, builtins->get("less_than_eq_i"));
    append_to_overloaded_function(FUNCS.less_than_eq, builtins->get("less_than_eq_f"));
    finish_building_overloaded_function(FUNCS.less_than_eq);

    FUNCS.greater_than = create_overloaded_function(builtins, "greater_than(any a,any b) -> bool");
    append_to_overloaded_function(FUNCS.greater_than, builtins->get("greater_than_i"));
    append_to_overloaded_function(FUNCS.greater_than, builtins->get("greater_than_f"));
    finish_building_overloaded_function(FUNCS.greater_than);

    FUNCS.greater_than_eq = create_overloaded_function(builtins, "greater_than_eq(any a,any b) -> bool");
    append_to_overloaded_function(FUNCS.greater_than_eq, builtins->get("greater_than_eq_i"));
    append_to_overloaded_function(FUNCS.greater_than_eq, builtins->get("greater_than_eq_f"));
    finish_building_overloaded_function(FUNCS.greater_than_eq);

    Term* max_func = create_overloaded_function(builtins, "max(any a,any b) -> any");
    append_to_overloaded_function(max_func, builtins->get("max_i"));
    append_to_overloaded_function(max_func, builtins->get("max_f"));
    finish_building_overloaded_function(max_func);

    Term* min_func = create_overloaded_function(builtins, "min(any a,any b) -> any");
    append_to_overloaded_function(min_func, builtins->get("min_i"));
    append_to_overloaded_function(min_func, builtins->get("min_f"));
    finish_building_overloaded_function(min_func);

    FUNCS.remainder = create_overloaded_function(builtins, "remainder(any a,any b) -> any");
    append_to_overloaded_function(FUNCS.remainder, builtins->get("remainder_i"));
    append_to_overloaded_function(FUNCS.remainder, builtins->get("remainder_f"));
    finish_building_overloaded_function(FUNCS.remainder);

    Term* mod_func = create_overloaded_function(builtins, "mod(any a,any b) -> any");
    append_to_overloaded_function(mod_func, builtins->get("mod_i"));
    append_to_overloaded_function(mod_func, builtins->get("mod_f"));
    finish_building_overloaded_function(mod_func);

    FUNCS.neg = create_overloaded_function(builtins, "neg(any n) -> any");
    append_to_overloaded_function(FUNCS.neg, builtins->get("neg_i"));
    append_to_overloaded_function(FUNCS.neg, builtins->get("neg_f"));
    finish_building_overloaded_function(FUNCS.neg);
    block_set_format_source_func(function_contents(FUNCS.neg), neg_function::formatSource);

    // Install native functions.
    module_patch_function(world->builtinPatch, "context", get_context);
    module_patch_function(world->builtinPatch, "file_version", file__version);
    module_patch_function(world->builtinPatch, "file_exists", file__exists);
    module_patch_function(world->builtinPatch, "file_read_text", file__read_text);
    module_patch_function(world->builtinPatch, "from_string", from_string);
    module_patch_function(world->builtinPatch, "set_context", set_context);
    module_patch_function(world->builtinPatch, "to_string_repr", to_string_repr);
    module_patch_function(world->builtinPatch, "test_spy", test_spy);
    module_patch_function(world->builtinPatch, "test_oracle", test_oracle);
    module_patch_function(world->builtinPatch, "reflect_this_block", reflect__this_block);
    module_patch_function(world->builtinPatch, "reflect_kernel", reflect__kernel);
    module_patch_function(world->builtinPatch, "sys_module_search_paths", sys__module_search_paths);
    module_patch_function(world->builtinPatch, "sys_perf_stats_reset", sys__perf_stats_reset);
    module_patch_function(world->builtinPatch, "sys_perf_stats_dump", sys__perf_stats_dump);
    module_patch_function(world->builtinPatch, "global_script_version", global_script_version);

    // Load the standard library from stdlib.ca
    parser::compile(builtins, parser::statement_list, STDLIB_CA_TEXT);

    closures_install_functions(builtins);
    modules_install_functions(builtins);
    reflection_install_functions(world->builtinPatch);
    interpreter_install_functions(world->builtinPatch);
    misc_builtins_setup_functions(world->builtinPatch);
    type_install_functions(builtins);

    native_patch_apply_patch(world->builtinPatch, builtins);

    block_set_format_source_func(function_contents(FUNCS.nonlocal), nonlocal_formatSource);

    // Fix 'builtins' module now that the module() function is created.
    change_function(builtinsTerm, FUNCS.module);
    block_set_bool_prop(builtins, sym_Builtins, true);

    // Fetch refereneces to certain stdlib funcs.
    ca_assert(FUNCS.declared_state != NULL);
    block_set_format_source_func(function_contents(FUNCS.declared_state),
        declared_state_format_source);

    FUNCS.has_effects = builtins->get("has_effects");
    block_set_has_effects(nested_contents(FUNCS.has_effects), true);

    function_contents(FUNCS.add)->overrides.specializeType = specializeType_add_sub_mult;
    function_contents(FUNCS.sub)->overrides.specializeType = specializeType_add_sub_mult;
    function_contents(FUNCS.mult)->overrides.specializeType = specializeType_add_sub_mult;
    function_contents(FUNCS.div)->overrides.specializeType = specializeType_div;

    FUNCS.get_with_symbol = builtins->get("get_with_symbol");
    FUNCS.length = builtins->get("length");
    FUNCS.list_append = builtins->get("List.append");
    FUNCS.native_patch = builtins->get("native_patch");
    FUNCS.not_func = builtins->get("not");
    FUNCS.output_explicit = builtins->get("output");
    FUNCS.type = builtins->get("type");

    FUNCS.memoize = builtins->get("memoize");
    block_set_evaluation_empty(function_contents(FUNCS.memoize), true);

    // Finish setting up types that are declared in stdlib.ca.
    TYPES.color = as_type(builtins->get("Color"));
    TYPES.file_signature = as_type(builtins->get("FileSignature"));
    TYPES.func = as_type(builtins->get("Func"));
    TYPES.module_ref = as_type(builtins->get("ModuleRef"));
    TYPES.stack = as_type(builtins->get("Stack"));
    TYPES.frame = as_type(builtins->get("Frame"));
    TYPES.module_frame = as_type(builtins->get("ModuleFrame"));
    TYPES.retained_frame = as_type(builtins->get("RetainedFrame"));
    TYPES.point = as_type(builtins->get("Point"));

    // Fix function_decl now that Func type is available.
    {
        change_declared_type(append_output_placeholder(function_contents(FUNCS.function_decl), NULL),
            TYPES.func);
        change_declared_type(FUNCS.function_decl, TYPES.func);
        finish_building_function(function_contents(FUNCS.function_decl));
    }

    TYPES.mutable_type = as_type(builtins->get("Mutable"));
    circa_setup_object_type(TYPES.mutable_type, sizeof(Value), Mutable_release);
    TYPES.mutable_type->initialize = Mutable_initialize;
    TYPES.mutable_type->toString = Mutable_toString;

    color_t::setup_type(TYPES.color);

    function_contents(FUNCS.list_append)->overrides.specializeType = List__append_specializeType;
}

void on_new_function_parsed(Term* func, caValue* functionName)
{
    // Catch certain builtin functions as soon as they are defined.
    #define STORE_BUILTIN_FUNC(ref, name) \
        if (ref == NULL && string_eq(functionName, name)) \
            ref = func;

    STORE_BUILTIN_FUNC(FUNCS.add, "add");
    STORE_BUILTIN_FUNC(FUNCS.closure_block, "closure_block");
    STORE_BUILTIN_FUNC(FUNCS.declared_state, "_declared_state");
    STORE_BUILTIN_FUNC(FUNCS.dynamic_term_eval, "_dynamic_term_eval");
    STORE_BUILTIN_FUNC(FUNCS.equals, "equals");
    STORE_BUILTIN_FUNC(FUNCS.func_call, "Func.call");
    STORE_BUILTIN_FUNC(FUNCS.div, "div");
    STORE_BUILTIN_FUNC(FUNCS.sub, "sub");
    STORE_BUILTIN_FUNC(FUNCS.mult, "mult");
    STORE_BUILTIN_FUNC(FUNCS.not_equals, "not_equals");
    STORE_BUILTIN_FUNC(FUNCS.nonlocal, "nonlocal");
}

CIRCA_EXPORT caWorld* circa_initialize()
{
    bootstrap_kernel();

    caWorld* world = global_world();

    Block* builtins = global_builtins_block();

    // Make sure there are no static errors in builtins. This shouldn't happen.
    if (has_static_errors(builtins)) {
        std::cout << "Static errors found in kernel:" << std::endl;
        dump(builtins);
        print_static_errors_formatted(builtins, std::cout);
    }

    // Load library paths from CIRCA_LIB_PATH
    const char* libPathEnv = getenv("CIRCA_LIB_PATH");
    if (libPathEnv != NULL) {
        Value libPathStr;
        set_string(&libPathStr, libPathEnv);

        Value libPaths;
        string_split(&libPathStr, ';', &libPaths);

        for (int i=0; i < list_length(&libPaths); i++) {
            caValue* path = list_get(&libPaths, i);
            if (string_eq(path, ""))
                continue;
            module_add_search_path(world, as_cstring(path));
        }
    }

    log_msg(0, "finished circa_initialize");

    world->bootstrapStatus = sym_Done;

    return world;
}

CIRCA_EXPORT void circa_shutdown(caWorld* world)
{
    symbol_deinitialize_global_table();

    world_uninitialize(world);

    delete world->root;
    world->root = NULL;

    for_each_root_type(predelete_type);
    for_each_root_type(delete_type);

    memset(&FUNCS, 0, sizeof(FUNCS));
    memset(&TYPES, 0, sizeof(TYPES));

    dealloc_world(world);
}

CIRCA_EXPORT caBlock* circa_kernel(caWorld* world)
{
    return world->root;
}

} // namespace circa
