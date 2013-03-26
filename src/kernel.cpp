// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"
#include "circa/file.h"

#include "actors.h"
#include "block.h"
#include "building.h"
#include "closures.h"
#include "control_flow.h"
#include "code_iterators.h"
#include "dict.h"
#include "function.h"
#include "gc.h"
#include "generic.h"
#include "hashtable.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "parser.h"
#include "reflection.h"
#include "selector.h"
#include "source_repro.h"
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
#include "types/callable.h"
#include "types/color.h"
#include "types/common.h"
#include "types/eval_context.h"
#include "types/indexable.h"
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

namespace cppbuild_function { void build_module(caStack*); }

Type* output_placeholder_specializeType(Term* caller)
{
    // Special case: if we're an accumulatingOutput then the output type is List.
    if (caller->boolProp("accumulatingOutput", false))
        return TYPES.list;

    if (caller->input(0) == NULL)
        return NULL;

    return declared_type(caller->input(0));
}

void hosted_assert(caStack* stack)
{
    if (!circa_bool_input(stack, 0))
        circa_output_error(stack, "Assert failed");
}

void file__exists(caStack* stack)
{
    set_bool(circa_output(stack, 0), circa_file_exists( circa_string_input(stack, 0)));
}
void file__version(caStack* stack)
{
    set_int(circa_output(stack, 0), circa_file_get_version(circa_string_input(stack, 0)));
}
void file__read_text(caStack* stack)
{
    circa_read_file(circa_string_input(stack, 0), circa_output(stack, 0));
}

void from_string(caStack* stack)
{
    parse_string_repr(circa_string_input(stack, 0), circa_output(stack, 0));
}

void to_string_repr(caStack* stack)
{
    write_string_repr(circa_input(stack, 0), circa_output(stack, 0));
}

void dynamic_method_call(caStack* stack)
{
    INCREMENT_STAT(DynamicMethodCall);

    caValue* args = circa_input(stack, 0);
    caValue* object = circa_index(args, 0);

    // Lookup method
    Term* term = (Term*) circa_caller_term(stack);
    std::string functionName = term->stringProp("syntax:functionName", "");

    // Find and dispatch method
    Term* method = find_method((Block*) circa_caller_block(stack),
        (Type*) circa_type_of(object), functionName.c_str());

    // copy(object, circa_output(stack, 1));

    if (method != NULL) {
        // Grab inputs before pop
        Value inputs;
        swap(args, &inputs);

        pop_frame(stack);
        push_frame_with_inputs(stack, function_contents(method), &inputs);
        return;
    }

    // Method not found. Raise error.
    std::string msg;
    msg += "Method ";
    msg += functionName;
    msg += " not found on type ";
    msg += as_cstring(&circa_type_of(object)->name);
    circa_output_error(stack, msg.c_str());
}

void send_func(caStack* stack)
{
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

void make_actor(caStack* stack)
{
    Block* block = as_block(circa_input(stack, 0));

    // Check that the block is well-defined for an actor.
    Term* primaryInput = get_input_placeholder(block, 0);
    circa::Value description;
    get_input_description(primaryInput, &description);

    if ...
        return raise_error_msg


    Actor* actor = create_actor(stack->world, block);
    set_actor(circa_output(stack, 0), actor);
}

void Actor__inject(caStack* stack)
{
    Actor* actor = as_actor(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);
    bool success = actor_inject(actor, name, val);
    set_bool(circa_output(stack, 0), success);
}

void Actor__call(caStack* stack)
{
    Actor* actor = as_actor(circa_input(stack, 0));

    if (actor_call_in_progress(actor)) {
        return raise_error_msg(stack, "Actor is in use; maybe this is a recursive call?");
    }

    caValue* input = circa_input(stack, 1);
    copy(input, actor_input_slot(actor));
    actor_run(actor);
    copy(actor_output_slot(actor), circa_output(stack, 0));
}

void Dict__count(caStack* stack)
{
    caValue* dict = circa_input(stack, 0);
    set_int(circa_output(stack, 0), dict_count(dict));
}

void Dict__set(caStack* stack)
{
    caValue* dict = circa_output(stack, 0);
    copy(circa_input(stack, 0), dict);

    const char* key = circa_string_input(stack, 1);
    caValue* value = circa_input(stack, 2);

    copy(value, dict_insert(dict, key));
}

void Dict__get(caStack* stack)
{
    caValue* dict = circa_input(stack, 0);
    const char* key = circa_string_input(stack, 1);

    copy(dict_get(dict, key), circa_output(stack, 0));
}

void Function__block(caStack* stack)
{
    Function* function = as_function(circa_input(stack, 0));
    set_block(circa_output(stack, 0), function_get_contents(function));
}

void empty_list(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    int size = circa_int_input(stack, 1);
    caValue* initialValue = circa_input(stack, 0);
    set_list(out, size);
    for (int i=0; i < size; i++) {
        copy(initialValue, list_get(out, i));
    }
}

void List__append(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);
    copy(circa_input(stack, 1), list_append(out));
}

void List__concat(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);

    caValue* additions = circa_input(stack, 1);

    int oldLength = list_length(out);
    int additionsLength = list_length(additions);

    list_resize(out, oldLength + additionsLength);
    for (int i = 0; i < additionsLength; i++)
        copy(list_get(additions, i), list_get(out, oldLength + i));
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

void List__resize(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);
    int count = circa_int_input(stack, 1);
    circa_resize(out, count);
}

void List__extend(caStack* stack)
{
    caValue* out = circa_output(stack, 1);
    copy(circa_input(stack, 0), out);

    caValue* additions = circa_input(stack, 1);

    int oldLength = list_length(out);
    int additionsLength = list_length(additions);

    list_resize(out, oldLength + additionsLength);
    for (int i = 0; i < additionsLength; i++)
        copy(list_get(additions, i), list_get(out, oldLength + i));
}

void List__count(caStack* stack)
{
    set_int(circa_output(stack, 0), list_length(circa_input(stack, 0)));
}
void List__length(caStack* stack)
{
    set_int(circa_output(stack, 0), list_length(circa_input(stack, 0)));
}

void List__insert(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);

    copy(circa_input(stack, 1), list_insert(out, circa_int_input(stack, 2)));
}

void List__slice(caStack* stack)
{
    caValue* input = circa_input(stack, 0);
    int start = circa_int_input(stack, 1);
    int end = circa_int_input(stack, 2);
    caValue* output = circa_output(stack, 0);

    if (start < 0)
        start = 0;
    else if (start > list_length(input))
        start = list_length(input);

    if (end > list_length(input))
        end = list_length(input);
    else if (end < 0)
        end = list_length(input) + end;

    if (end < start) {
        set_list(output, 0);
        return;
    }

    int length = end - start;
    set_list(output, length);

    for (int i=0; i < length; i++)
        copy(list_get(input, start + i), list_get(output, i));
}

void List__join(caStack* stack)
{
    caValue* input = circa_input(stack, 0);
    caValue* joiner = circa_input(stack, 1);

    caValue* out = circa_output(stack, 0);
    set_string(out, "");

    for (int i=0; i < list_length(input); i++) {
        if (i != 0)
            string_append(out, joiner);

        string_append(out, list_get(input, i));
    }
}

void List__get(caStack* stack)
{
    caValue* self = circa_input(stack, 0);
    int index = circa_int_input(stack, 1);
    if (index < 0 || index >= list_length(self))
        return raise_error_msg(stack, "Index out of bounds");

    copy(list_get(self, index), circa_output(stack, 0));
}

void List__set(caStack* stack)
{
    caValue* self = circa_output(stack, 0);
    copy(circa_input(stack, 0), self);

    int index = circa_int_input(stack, 1);
    caValue* value = circa_input(stack, 2);

    touch(self);
    copy(value, list_get(self, index));
}

void Map__contains(caStack* stack)
{
    caValue* value = hashtable_get(circa_input(stack, 0), circa_input(stack, 1));
    set_bool(circa_output(stack, 0), value != NULL);
}

void Map__remove(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);

    hashtable_remove(out, circa_input(stack, 1));
}

void Map__get(caStack* stack)
{
    caValue* table = circa_input(stack, 0);
    caValue* key = circa_input(stack, 1);
    caValue* value = hashtable_get(table, key);
    if (value == NULL) {
        std::string msg = "Key not found: " + to_string(key);
        return circa_output_error(stack, msg.c_str());
    }
    copy(value, circa_output(stack, 0));
}

void Map__set(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);

    caValue* key = circa_input(stack, 1);
    caValue* value = circa_input(stack, 2);

    copy(value, hashtable_insert(out, key, false));
}

void Map__insertPairs(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);

    caValue* pairs = circa_input(stack, 1);
    for (int i=0; i < list_length(pairs); i++) {
        caValue* pair = list_get(pairs, i);
        copy(list_get(pair, 1), hashtable_insert(out, list_get(pair, 0), false));
    }
}

void Mutable__get(caStack* stack)
{
    caValue* val = (caValue*) circa_object_input(stack, 0);
    copy(val, circa_output(stack, 0));
}

void Mutable__set(caStack* stack)
{
    caValue* val = (caValue*) circa_object_input(stack, 0);
    copy(circa_input(stack, 1), val);
}

void MutableInitialize(Type* type, caValue* value)
{
    object_initialize(type, value);
    caValue* val = (caValue*) object_get_body(value);
    initialize_null(val);
}

void MutableRelease(void* object)
{
    caValue* val = (caValue*) object;
    set_null(val);
}

void String__char_at(caStack* stack)
{
    const char* str = circa_string_input(stack, 0);
    int index = circa_int_input(stack, 1);

    if (index < 0) {
        circa_output_error(stack, "negative index");
        return;
    }

    if ((unsigned) index >= strlen(str)) {
        set_string(circa_output(stack, 0), "");
        return;
    }

    char output[1];
    output[0] = str[index];
    set_string(circa_output(stack, 0), output, 1);
}

void String__length(caStack* stack)
{
    const char* str = circa_string_input(stack, 0);
    set_int(circa_output(stack, 0), (int) strlen(str));
}

void String__substr(caStack* stack)
{
    int start = circa_int_input(stack, 1);
    int end = circa_int_input(stack, 2);
    std::string const& s = as_string(circa_input(stack, 0));

    if (start < 0) return circa_output_error(stack, "Negative index");
    if (end < 0) return circa_output_error(stack, "Negative index");

    if ((unsigned) start > s.length()) {
        std::stringstream msg;
        msg << "Start index is too high: " << start;
        return circa_output_error(stack, msg.str().c_str());
    }
    if ((unsigned) (start+end) > s.length()) {
        std::stringstream msg;
        msg << "End index is too high: " << start;
        return circa_output_error(stack, msg.str().c_str());
    }

    set_string(circa_output(stack, 0), s.substr(start, end));
}

char character_to_lower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c + 'a' - 'A';
    return c;
}

void String__to_camel_case(caStack* stack)
{
    const char* in = circa_string_input(stack, 0);
    set_string(circa_output(stack, 0), in);

    char* out = (char*) as_cstring(circa_output(stack, 0));
    if (out[0] == 0)
        return;

    out[0] = character_to_lower(out[0]);
}

void String__to_lower(caStack* stack)
{
    const char* in = circa_string_input(stack, 0);
    int len = (int) strlen(in);

    set_string(circa_output(stack, 0), in);
    char* out = (char*) as_cstring(circa_output(stack, 0));

    for (int i=0; i < len; i++) {
        char c = in[i];

        if (c >= 'A' && c <= 'Z')
            c = c + 'a' - 'A';
        out[i] = c;
    }
}

void String__to_upper(caStack* stack)
{
    const char* in = circa_string_input(stack, 0);
    int len = (int) strlen(in);

    set_string(circa_output(stack, 0), in);
    char* out = (char*) as_cstring(circa_output(stack, 0));

    for (int i=0; i < len; i++) {
        char c = in[i];

        if (c >= 'a' && c <= 'z')
            c = c + 'A' - 'a';
        out[i] = c;
    }
}

void String__slice(caStack* stack)
{
    int start = circa_int_input(stack, 1);
    int end = circa_int_input(stack, 2);
    std::string const& s = as_string(circa_input(stack, 0));

    // Negative indexes are relatve to end of string
    if (start < 0) start = (int) s.length() + start;
    if (end < 0) end = (int) s.length() + end;

    if (start < 0) return set_string(circa_output(stack, 0), "");
    if (end < 0) return set_string(circa_output(stack, 0), "");

    if ((unsigned) start > s.length())
        start = (int) s.length();

    if ((unsigned) end > s.length())
        end = (int) s.length();

    if (end < start)
        return set_string(circa_output(stack, 0), "");

    set_string(circa_output(stack, 0), s.substr(start, end - start));
}

void String__ends_with(caStack* stack)
{
    set_bool(circa_output(stack, 0), string_ends_with(circa_input(stack, 0), as_cstring(circa_input(stack, 1))));
}
void String__starts_with(caStack* stack)
{
    set_bool(circa_output(stack, 0), string_starts_with(circa_input(stack, 0), as_cstring(circa_input(stack, 1))));
}

void String__split(caStack* stack)
{
    string_split(circa_input(stack, 0), string_get(circa_input(stack, 1), 0), circa_output(stack, 0));
}

void Type__name(caStack* stack)
{
    Type* type = as_type(circa_input(stack, 0));
    copy(&type->name, circa_output(stack, 0));
}

void Type__property(caStack* stack)
{
    Type* type = as_type(circa_input(stack, 0));
    const char* str = as_cstring(circa_input(stack, 1));
    caValue* prop = get_type_property(type, str);
    if (prop == NULL)
        set_null(circa_output(stack, 0));
    else
        copy(prop, circa_output(stack, 0));
}

void Type__declaringTerm(caStack* stack)
{
    Type* type = as_type(circa_input(stack, 0));
    set_term_ref(circa_output(stack, 0), type->declaringTerm);
}

void typeof_func(caStack* stack)
{
    caValue* in = circa_input(stack, 0);
    set_type(circa_output(stack, 0), in->value_type);
}

void static_type_func(caStack* stack)
{
    Term* caller = (Term*) circa_caller_term(stack);
    Term* input = caller->input(0);
    set_type(circa_output(stack, 0), input->type);
}

void length(caStack* stack)
{
    set_int(circa_output(stack, 0), num_elements(circa_input(stack, 0)));
}

std::string stackVariable_toString(caValue* value)
{
    short relativeFrame = value->value_data.asint >> 16;
    short index = (value->value_data.asint & 0xffff);
    std::stringstream strm;
    strm << "[frame:" << relativeFrame << ", index:" << index << "]";
    return strm.str();
}

World* global_world()
{
    return g_world;
}

Block* global_root_block()
{
    return global_world()->root;
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
    copy(circa_input(stack, 0), list_append(g_spyValues));
}

void section_block_formatSource(caValue* source, Term* term)
{
    format_name_binding(source, term);
    append_phrase(source, "section", term, sym_None);
    append_phrase(source, " ", term, sym_Whitespace);
    format_block_source(source, nested_contents(term), term);
}

void bootstrap_kernel()
{
    // First, instanciate the types that are used by Type.
    TYPES.dict = create_type_uninitialized();
    TYPES.null = create_type_uninitialized();
    TYPES.string = create_type_uninitialized();
    TYPES.type = create_type_uninitialized();

    initialize_type(TYPES.dict);
    initialize_type(TYPES.null);
    initialize_type(TYPES.string);
    initialize_type(TYPES.type);
    string_setup_type(TYPES.string);

    // Initialize remaining global types.
    TYPES.any = create_type();
    TYPES.block = create_type();
    TYPES.bool_type = create_type();
    TYPES.error = create_type();
    TYPES.eval_context = create_type();
    TYPES.float_type = create_type();
    TYPES.function = create_type();
    TYPES.int_type = create_type();
    TYPES.list = create_type();
    TYPES.map = create_type();
    TYPES.opaque_pointer = create_type();
    TYPES.symbol = create_type();
    TYPES.term = create_type();
    TYPES.void_type = create_type();

    any_t::setup_type(TYPES.any);
    block_setup_type(TYPES.block);
    bool_t::setup_type(TYPES.bool_type);
    dict_t::setup_type(TYPES.dict);
    eval_context_t::setup_type(TYPES.eval_context);
    function_t::setup_type(TYPES.function);
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

    // Start building World
    g_world = alloc_world();
    g_world->bootstrapStatus = sym_Bootstrapping;

    // Create root Block.
    g_world->root = new Block();
    Block* kernel = g_world->root;

    // Create value function
    Term* valueFunc = kernel->appendNew();
    rename(valueFunc, "value");
    FUNCS.value = valueFunc;

    // Create Type type
    Term* typeType = kernel->appendNew();
    typeType->function = FUNCS.value;
    typeType->type = TYPES.type;
    term_value(typeType)->value_type = TYPES.type;
    term_value(typeType)->value_data.ptr = TYPES.type;
    TYPES.type->declaringTerm = typeType;
    rename(typeType, "Type");

    // Create Any type
    Term* anyType = kernel->appendNew();
    anyType->function = valueFunc;
    anyType->type = TYPES.type;
    term_value(anyType)->value_type = TYPES.type;
    term_value(anyType)->value_data.ptr = TYPES.any;
    TYPES.any->declaringTerm = anyType;
    rename(anyType, "any");

    // Create Function type
    Term* functionType = kernel->appendNew();
    functionType->function = valueFunc;
    functionType->type = TYPES.type;
    TYPES.function->declaringTerm = functionType;
    term_value(functionType)->value_type = TYPES.type;
    term_value(functionType)->value_data.ptr = TYPES.function;
    rename(functionType, "Function");

    // Initialize value() func
    valueFunc->type = TYPES.function;
    valueFunc->function = valueFunc;
    make(TYPES.function, term_value(valueFunc));

    function_t::initialize(TYPES.function, term_value(valueFunc));
    initialize_function(valueFunc);
    as_function(valueFunc)->name = "value";
    block_set_evaluation_empty(function_contents(valueFunc), true);

    // Initialize primitive types (this requires value() function)
    create_type_value(kernel, TYPES.bool_type, "bool");
    create_type_value(kernel, TYPES.block, "Block");
    create_type_value(kernel, TYPES.dict, "Dict");
    create_type_value(kernel, TYPES.float_type, "number");
    create_type_value(kernel, TYPES.int_type, "int");
    create_type_value(kernel, TYPES.list, "List");
    create_type_value(kernel, TYPES.opaque_pointer, "opaque_pointer");
    create_type_value(kernel, TYPES.string, "String");
    create_type_value(kernel, TYPES.symbol, "Symbol");
    create_type_value(kernel, TYPES.term, "Term");
    create_type_value(kernel, TYPES.void_type, "void");
    create_type_value(kernel, TYPES.map, "Map");

    // Finish initializing World (this requires List and Hashtable types)
    world_initialize(g_world);

    // Create global symbol table (requires Hashtable type)
    symbol_initialize_global_table();

    // Setup output_placeholder() function, needed to declare functions properly.
    FUNCS.output = create_value(kernel, TYPES.function, "output_placeholder");
    function_t::initialize(TYPES.function, term_value(FUNCS.output));
    initialize_function(FUNCS.output);
    as_function(FUNCS.output)->name = "output_placeholder";
    as_function(FUNCS.output)->evaluate = NULL;
    as_function(FUNCS.output)->specializeType = output_placeholder_specializeType;
    ca_assert(function_get_output_type(FUNCS.output, 0) == TYPES.any);

    // Fix some holes in value() function
    {
        Function* attrs = as_function(valueFunc);
        Term* output = append_output_placeholder(function_contents(attrs), NULL);
        change_declared_type(output, TYPES.any);
        finish_building_function(function_contents(attrs));
    }

    ca_assert(function_get_output_type(valueFunc, 0) == TYPES.any);

    // input_placeholder() is needed before we can declare a function with inputs
    FUNCS.input = import_function(kernel, NULL, "input_placeholder() -> any");
    block_set_evaluation_empty(function_contents(FUNCS.input), true);

    // Now that we have input_placeholder() let's declare one input on output_placeholder()
    apply(function_contents(as_function(FUNCS.output)),
        FUNCS.input, TermList())->setBoolProp("optional", true);

    namespace_function::early_setup(kernel);

    // Setup declare_field() function, needed to represent compound types.
    FUNCS.declare_field = import_function(kernel, NULL, "declare_field() -> any");

    // Initialize a few more types
    Term* set_type = create_value(kernel, TYPES.type, "Set");
    set_t::setup_type(unbox_type(set_type));

    Term* indexableType = create_value(kernel, TYPES.type, "Indexable");
    indexable_t::setup_type(unbox_type(indexableType));

    TYPES.selector = unbox_type(create_value(kernel, TYPES.type, "Selector"));
    list_t::setup_type(TYPES.selector);

    control_flow_setup_funcs(kernel);
    selector_setup_funcs(kernel);
    loop_setup_functions(kernel);

    // Setup all the builtin functions defined in src/functions
    setup_builtin_functions(kernel);

    FUNCS.section_block = import_function(kernel, NULL, "def section_block() -> any");
    as_function(FUNCS.section_block)->formatSource = section_block_formatSource;

    // Create IMPLICIT_TYPES (deprecated)
    type_initialize_kernel(kernel);

    // Now we can build derived functions

    // Create overloaded functions
    FUNCS.add = create_overloaded_function(kernel, "add(any a,any b) -> any");
    append_to_overloaded_function(FUNCS.add, FUNCS.add_i);
    append_to_overloaded_function(FUNCS.add, FUNCS.add_f);

    Term* less_than = create_overloaded_function(kernel, "less_than(any a,any b) -> bool");
    append_to_overloaded_function(less_than, kernel->get("less_than_i"));
    append_to_overloaded_function(less_than, kernel->get("less_than_f"));

    Term* less_than_eq = create_overloaded_function(kernel, "less_than_eq(any a,any b) -> bool");
    append_to_overloaded_function(less_than_eq, kernel->get("less_than_eq_i"));
    append_to_overloaded_function(less_than_eq, kernel->get("less_than_eq_f"));

    Term* greater_than = create_overloaded_function(kernel, "greater_than(any a,any b) -> bool");
    append_to_overloaded_function(greater_than, kernel->get("greater_than_i"));
    append_to_overloaded_function(greater_than, kernel->get("greater_than_f"));

    Term* greater_than_eq = create_overloaded_function(kernel, "greater_than_eq(any a,any b) -> bool");
    append_to_overloaded_function(greater_than_eq, kernel->get("greater_than_eq_i"));
    append_to_overloaded_function(greater_than_eq, kernel->get("greater_than_eq_f"));

    Term* max_func = create_overloaded_function(kernel, "max(any a,any b) -> any");
    append_to_overloaded_function(max_func, kernel->get("max_i"));
    append_to_overloaded_function(max_func, kernel->get("max_f"));

    Term* min_func = create_overloaded_function(kernel, "min(any a,any b) -> any");
    append_to_overloaded_function(min_func, kernel->get("min_i"));
    append_to_overloaded_function(min_func, kernel->get("min_f"));

    Term* remainder_func = create_overloaded_function(kernel, "remainder(any a,any b) -> any");
    append_to_overloaded_function(remainder_func, kernel->get("remainder_i"));
    append_to_overloaded_function(remainder_func, kernel->get("remainder_f"));

    Term* mod_func = create_overloaded_function(kernel, "mod(any a,any b) -> any");
    append_to_overloaded_function(mod_func, kernel->get("mod_i"));
    append_to_overloaded_function(mod_func, kernel->get("mod_f"));

    FUNCS.mult = create_overloaded_function(kernel, "mult(any a,any b) -> any");
    append_to_overloaded_function(FUNCS.mult, kernel->get("mult_i"));
    append_to_overloaded_function(FUNCS.mult, kernel->get("mult_f"));

    FUNCS.neg = create_overloaded_function(kernel, "neg(any n) -> any");
    append_to_overloaded_function(FUNCS.neg, kernel->get("neg_i"));
    append_to_overloaded_function(FUNCS.neg, kernel->get("neg_f"));
    as_function(FUNCS.neg)->formatSource = neg_function::formatSource;

    FUNCS.sub = create_overloaded_function(kernel, "sub(any a,any b) -> any");
    append_to_overloaded_function(FUNCS.sub, kernel->get("sub_i"));
    append_to_overloaded_function(FUNCS.sub, kernel->get("sub_f"));

    // Create vectorized functions
    Term* add_v = create_function(kernel, "add_v");
    create_function_vectorized_vv(function_contents(add_v), FUNCS.add, TYPES.list, TYPES.list);
    Term* add_s = create_function(kernel, "add_s");
    create_function_vectorized_vs(function_contents(add_s), FUNCS.add, TYPES.list, TYPES.any);

    append_to_overloaded_function(FUNCS.add, add_v);
    append_to_overloaded_function(FUNCS.add, add_s);

    Term* sub_v = create_function(kernel, "sub_v");
    create_function_vectorized_vv(function_contents(sub_v), FUNCS.sub, TYPES.list, TYPES.list);
    Term* sub_s = create_function(kernel, "sub_s");
    create_function_vectorized_vs(function_contents(sub_s), FUNCS.sub, TYPES.list, TYPES.any);
    
    append_to_overloaded_function(FUNCS.sub, sub_v);
    append_to_overloaded_function(FUNCS.sub, sub_s);

    // Create vectorized mult() functions
    Term* mult_v = create_function(kernel, "mult_v");
    create_function_vectorized_vv(function_contents(mult_v), FUNCS.mult, TYPES.list, TYPES.list);
    Term* mult_s = create_function(kernel, "mult_s");
    create_function_vectorized_vs(function_contents(mult_s), FUNCS.mult, TYPES.list, TYPES.any);

    append_to_overloaded_function(FUNCS.mult, mult_v);
    append_to_overloaded_function(FUNCS.mult, mult_s);

    Term* div_s = create_function(kernel, "div_s");
    create_function_vectorized_vs(function_contents(div_s), FUNCS.div, TYPES.list, TYPES.any);

    // Need dynamic_method before any hosted functions
    FUNCS.dynamic_method = import_function(kernel, dynamic_method_call,
            "def dynamic_method(any inputs :multiple) -> any");

    // Load the standard library from stdlib.ca
    parser::compile(kernel, parser::statement_list, STDLIB_CA_TEXT);

    // Install C functions
    static const ImportRecord records[] = {
        {"assert", hosted_assert},
        {"cppbuild:build_module", cppbuild_function::build_module},
        {"file:version", file__version},
        {"file:exists", file__exists},
        {"file:read_text", file__read_text},
        {"length", length},
        {"from_string", from_string},
        {"to_string_repr", to_string_repr},
        {"send", send_func},
        {"test_spy", test_spy},
        {"test_oracle", test_oracle},
        {"refactor:rename", refactor__rename},
        {"refactor:change_function", refactor__change_function},
        {"reflect:this_block", reflect__this_block},
        {"reflect:kernel", reflect__kernel},
        {"sys:module_search_paths", sys__module_search_paths},
        {"sys:perf_stats_reset", sys__perf_stats_reset},
        {"sys:perf_stats_dump", sys__perf_stats_dump},

        {"make_actor", make_actor},
        {"Actor.inject", Actor__inject},
        {"Actor.call", Actor__call},

        {"Dict.count", Dict__count},
        {"Dict.get", Dict__get},
        {"Dict.set", Dict__set},

        {"Function.block", Function__block},

        {"empty_list", empty_list},
        {"List.append", List__append},
        {"List.concat", List__concat},
        {"List.resize", List__resize},
        {"List.count", List__count},
        {"List.insert", List__insert},
        {"List.length", List__length},
        {"List.join", List__join},
        {"List.slice", List__slice},
        {"List.get", List__get},
        {"List.set", List__set},

        {"Map.contains", Map__contains},
        {"Map.remove", Map__remove},
        {"Map.get", Map__get},
        {"Map.set", Map__set},
        {"Map.insertPairs", Map__insertPairs},

        {"Mutable.get", Mutable__get},
        {"Mutable.set", Mutable__set},

        {"String.char_at", String__char_at},
        {"String.ends_with", String__ends_with},
        {"String.length", String__length},
        {"String.substr", String__substr},
        {"String.slice", String__slice},
        {"String.starts_with", String__starts_with},
        {"String.split", String__split},
        {"String.to_camel_case", String__to_camel_case},
        {"String.to_upper", String__to_upper},
        {"String.to_lower", String__to_lower},
        
        {"Type.name", Type__name},
        {"Type.property", Type__property},
        {"Type.declaringTerm", Type__declaringTerm},
        {"type", typeof_func},
        {"static_type", static_type_func},

        {NULL, NULL}
    };

    install_function_list(kernel, records);

    closures_install_functions(kernel);
    modules_install_functions(kernel);
    reflection_install_functions(kernel);
    interpreter_install_functions(kernel);

    // Fetch refereneces to certain builtin funcs.
    FUNCS.block_dynamic_call = kernel->get("Block.call");
    FUNCS.dll_patch = kernel->get("sys:dll_patch");
    FUNCS.dynamic_call = kernel->get("dynamic_call");
    FUNCS.has_effects = kernel->get("has_effects");
    FUNCS.length = kernel->get("length");
    FUNCS.list_append = kernel->get("List.append");
    FUNCS.native_patch = kernel->get("native_patch");
    FUNCS.not_func = kernel->get("not");
    FUNCS.output_explicit = kernel->get("output");
    FUNCS.type = kernel->get("type");

    block_set_has_effects(nested_contents(FUNCS.has_effects), true);

    // Finish setting up types that were declared in stdlib.ca.
    TYPES.actor = as_type(kernel->get("Actor"));
    actor_setup_type(TYPES.actor);
    TYPES.color = as_type(kernel->get("Color"));
    TYPES.closure = as_type(kernel->get("Closure"));
    callable_t::setup_type(as_type(kernel->get("Callable")));
    TYPES.frame = as_type(kernel->get("Frame"));
    TYPES.point = as_type(kernel->get("Point"));
    TYPES.file_signature = as_type(kernel->get("FileSignature"));

    Type* mutableType = as_type(kernel->get("Mutable"));
    circa_setup_object_type(mutableType, sizeof(Value), MutableRelease);
    mutableType->initialize = MutableInitialize;

    color_t::setup_type(TYPES.color);

    as_function(FUNCS.list_append)->specializeType = List__append_specializeType;
}

CIRCA_EXPORT caWorld* circa_initialize()
{
    memset(&FUNCS, 0, sizeof(FUNCS));
    memset(&TYPES, 0, sizeof(TYPES));

    bootstrap_kernel();

    caWorld* world = global_world();

    Block* kernel = global_root_block();

    // Make sure there are no static errors in the kernel. This shouldn't happen.
    if (has_static_errors(kernel)) {
        std::cout << "Static errors found in kernel:" << std::endl;
        print_static_errors_formatted(kernel, std::cout);
        internal_error("circa fatal: static errors found in kernel");
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

    delete world->root;
    world->root = NULL;

    memset(&FUNCS, 0, sizeof(FUNCS));

    gc_collect();

    free(world);
}

CIRCA_EXPORT caBlock* circa_kernel(caWorld* world)
{
    return world->root;
}

} // namespace circa
