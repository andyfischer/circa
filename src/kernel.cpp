// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"
#include "circa/file.h"

#include "branch.h"
#include "building.h"
#include "control_flow.h"
#include "code_iterators.h"
#include "dict.h"
#include "evaluation.h"
#include "function.h"
#include "gc.h"
#include "generic.h"
#include "importing.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "parser.h"
#include "reflection.h"
#include "selector.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "static_checking.h"
#include "string_type.h"
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
#include "types/hashtable.h"
#include "types/indexable.h"
#include "types/int.h"
#include "types/name.h"
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
void setup_builtin_functions(Branch*);

bool STATIC_INITIALIZATION_FINISHED = false;
bool FINISHED_BOOTSTRAP = false;
bool SHUTTING_DOWN = false;

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* DICT_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* NAME_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* VOID_TYPE = NULL;
Term* OPAQUE_POINTER_TYPE = NULL;

// Structure of pointers to builtin functions.
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

Value TrueValue;
Value FalseValue;

List g_oracleValues;
List g_spyValues;

namespace cppbuild_function { void build_module(caStack*); }

Type* output_placeholder_specializeType(Term* caller)
{
    // Special case: if we're an accumulatingOutput then the output type is List.
    if (caller->boolProp("accumulatingOutput", false))
        return &LIST_T;

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
    circa_parse_string(circa_string_input(stack, 0), circa_output(stack, 0));
}

void to_string_repr(caStack* stack)
{
    circa_to_string_repr(circa_input(stack, 0), circa_output(stack, 0));
}

void call_actor_func(caStack* stack)
{
    const char* actorName = circa_string_input(stack, 0);
    caValue* msg = circa_input(stack, 1);

    if (stack->world == NULL) {
        circa_output_error(stack, "Stack was not created with World");
        return;
    }

    circa_actor_run_message(stack->world, actorName, msg);
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
    Term* method = find_method((Branch*) circa_caller_branch(stack),
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
    msg += name_to_string(circa_type_of(object)->name);
    circa_output_error(stack, msg.c_str());
}

void send_func(caStack* stack)
{
    const char* actorName = circa_string_input(stack, 0);
    caValue* msg = circa_input(stack, 1);

    if (stack->world == NULL) {
        circa_output_error(stack, "Stack was not created with World");
        return;
    }

    ListData* actor = find_actor(stack->world, actorName);

    if (actor == NULL) {
        std::string msg = "Actor not found: ";
        msg += actorName;
        circa_output_error(stack, msg.c_str());
        return;
    }

    actor_send_message(actor, msg);
}

void refactor__rename(caStack* stack)
{
    rename(as_term_ref(circa_input(stack, 0)),
            name_from_string(circa_input(stack, 1)));
}

void refactor__change_function(caStack* stack)
{
    change_function(as_term_ref(circa_input(stack, 0)),
        (Term*) circa_caller_input_term(stack, 1));
}

void reflect__this_branch(caStack* stack)
{
    set_branch(circa_output(stack, 0), (Branch*) circa_caller_branch(stack));
}

void reflect__kernel(caStack* stack)
{
    set_branch(circa_output(stack, 0), global_root_branch());
}

void sys__module_search_paths(caStack* stack)
{
    copy(modules_get_search_paths(), circa_output(stack, 0));
}
void sys__perf_stats_reset(caStack* stack)
{
    perf_stats_reset();
}
void sys__perf_stats_dump(caStack* stack)
{
    perf_stats_to_list(circa_output(stack, 0));
}

void load_module(caStack* stack)
{
    const char* filename = circa_string_input(stack, 0);
    
    Branch* branch = load_module_from_file(filename, filename);

    set_branch(circa_output(stack, 0), branch);
}

void Dict__count(caStack* stack)
{
    caValue* dict = circa_input(stack, 0);
    set_int(circa_output(stack, 0), dict_count(dict));
}

void Dict__set(caStack* stack)
{
    caValue* dict = circa_output(stack, 1);
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

void Function__branch(caStack* stack)
{
    Function* function = as_function(circa_input(stack, 0));
    set_branch(circa_output(stack, 0), function_get_contents(function));
}

void List__append(caStack* stack)
{
    caValue* out = circa_output(stack, 1);
    copy(circa_input(stack, 0), out);
    copy(circa_input(stack, 1), list_append(out));
}

Type* List__append_specializeType(Term* term)
{
    Term* listInput = term->input(0);
    switch (list_get_parameter_type(&listInput->type->parameter)) {
    case LIST_UNTYPED:
        return listInput->type;
    case LIST_TYPED_UNSIZED:
    {
        Type* listElementType = list_get_repeated_type_from_type(listInput->type);
        Type* commonType = find_common_type(listElementType, term->input(1)->type);
        if (commonType == listElementType)
            return listInput->type;
        else
            return create_typed_unsized_list_type(commonType);
    }
    case LIST_TYPED_SIZED:
    case LIST_TYPED_SIZED_NAMED:
    {    
        List elementTypes;
        copy(list_get_type_list_from_type(listInput->type), &elementTypes);
        set_type(elementTypes.append(), term->input(1)->type);
        return create_typed_unsized_list_type(find_common_type(&elementTypes));
    }
    case LIST_INVALID_PARAMETER:
    default:
        return &ANY_T;
    }
}

void List__resize(caStack* stack)
{
    caValue* out = circa_output(stack, 1);
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
    caValue* out = circa_output(stack, 1);
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
    caValue* index = circa_input(stack, 1);
    copy(list_get(self, as_int(index)), circa_output(stack, 0));
}

void Map__contains(caStack* stack)
{
    caValue* value = hashtable_t::get_value(circa_input(stack, 0), circa_input(stack, 1));
    set_bool(circa_output(stack, 0), value != NULL);
}

void Map__set(caStack* stack)
{
    caValue* out = circa_output(stack, 1);
    copy(circa_input(stack, 0), out);

    caValue* key = circa_input(stack, 1);
    caValue* value = circa_input(stack, 2);

    copy(value, hashtable_t::table_insert(out, key, false));
}

void Map__remove(caStack* stack)
{
    caValue* out = circa_output(stack, 1);
    copy(circa_input(stack, 0), out);

    hashtable_t::table_remove(out, circa_input(stack, 1));
}

void Map__get(caStack* stack)
{
    caValue* table = circa_input(stack, 0);
    caValue* key = circa_input(stack, 1);
    caValue* value = hashtable_t::get_value(table, key);
    if (value == NULL) {
        std::string msg = "Key not found: " + to_string(key);
        return circa_output_error(stack, msg.c_str());
    }
    copy(value, circa_output(stack, 0));
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
    set_int(circa_output(stack, 0), strlen(str));
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
    int len = strlen(in);

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
    int len = strlen(in);

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
    if (start < 0) start = s.length() + start;
    if (end < 0) end = s.length() + end;

    if (start < 0) return set_string(circa_output(stack, 0), "");
    if (end < 0) return set_string(circa_output(stack, 0), "");

    if ((unsigned) start > s.length())
        start = s.length();

    if ((unsigned) end > s.length())
        end = s.length();

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
    set_string(circa_output(stack, 0), name_to_string(as_type(circa_input(stack, 0))->name));
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

Branch* global_root_branch()
{
    return global_world()->root;
}

std::string ref_toString(caValue* val)
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

int ref_hashFunc(caValue* val)
{
    Term* term = as_term_ref(val);
    if (term == NULL)
        return 0;
    return term->id;
}

void ref_setup_type(Type* type)
{
    type->name = name_from_string("Term");
    type->storageType = STORAGE_TYPE_REF;
    type->toString = ref_toString;
    type->hashFunc = ref_hashFunc;
}

// Spy & oracle
void test_spy_clear()
{
    set_list(&g_spyValues, 0);
}
List* test_spy_get_results()
{
    return &g_spyValues;
}
void test_oracle_clear()
{
    set_list(&g_oracleValues, 0);
}
void test_oracle_send(caValue* value)
{
    copy(value, list_append(&g_oracleValues));
}
void test_oracle_send(int i)
{
    set_int(list_append(&g_oracleValues), i);
}
void test_oracle(caStack* stack)
{
    if (g_oracleValues.length() == 0)
        set_null(circa_output(stack, 0));
    else {
        copy(g_oracleValues[0], circa_output(stack, 0));
        g_oracleValues.remove(0);
    }
}

void test_spy(caStack* stack)
{
    copy(circa_input(stack, 0), g_spyValues.append());
}

void bootstrap_kernel()
{
    // Create global World.
    g_world = (World*) malloc(sizeof(*g_world));

    // Initialize global type objects
    null_t::setup_type(&NULL_T);
    bool_t::setup_type(&BOOL_T);
    branch_setup_type(&BRANCH_T);
    dict_t::setup_type(&DICT_T);
    eval_context_t::setup_type(&EVAL_CONTEXT_T);
    number_t::setup_type(&FLOAT_T);
    int_t::setup_type(&INT_T);
    list_t::setup_type(&LIST_T);
    name_t::setup_type(&NAME_T);
    opaque_pointer_t::setup_type(&OPAQUE_POINTER_T);
    ref_setup_type(&REF_T);
    string_setup_type(&STRING_T);
    void_t::setup_type(&VOID_T);
    eval_context_setup_type(&EVAL_CONTEXT_T);
    string_setup_type(&ERROR_T); // errors are just stored as strings for now

    // Create root Branch.
    g_world->root = new Branch();
    Branch* kernel = g_world->root;

    // Create value function
    Term* valueFunc = kernel->appendNew();
    rename(valueFunc, name_from_string("value"));
    FUNCS.value = valueFunc;

    // Create Type type
    TYPE_TYPE = kernel->appendNew();
    TYPE_TYPE->function = FUNCS.value;
    TYPE_TYPE->type = &TYPE_T;
    term_value(TYPE_TYPE)->value_type = &TYPE_T;
    term_value(TYPE_TYPE)->value_data.ptr = &TYPE_T;
    TYPE_T.declaringTerm = TYPE_TYPE;
    type_t::setup_type(&TYPE_T);
    rename(TYPE_TYPE, name_from_string("Type"));

    // Create Any type
    ANY_TYPE = kernel->appendNew();
    ANY_TYPE->function = valueFunc;
    ANY_TYPE->type = &TYPE_T;
    term_value(ANY_TYPE)->value_type = &TYPE_T;
    term_value(ANY_TYPE)->value_data.ptr = &ANY_T;
    ANY_T.declaringTerm = ANY_TYPE;
    any_t::setup_type(&ANY_T);
    rename(ANY_TYPE, name_from_string("any"));

    // Create Function type
    function_t::setup_type(&FUNCTION_T);
    FUNCTION_TYPE = kernel->appendNew();
    FUNCTION_TYPE->function = valueFunc;
    FUNCTION_TYPE->type = &TYPE_T;
    FUNCTION_T.declaringTerm = FUNCTION_TYPE;
    term_value(FUNCTION_TYPE)->value_type = &TYPE_T;
    term_value(FUNCTION_TYPE)->value_data.ptr = &FUNCTION_T;
    rename(FUNCTION_TYPE, name_from_string("Function"));

    // Initialize value() func
    valueFunc->type = &FUNCTION_T;
    valueFunc->function = valueFunc;
    make(&FUNCTION_T, term_value(valueFunc));

    function_t::initialize(&FUNCTION_T, term_value(valueFunc));
    initialize_function(valueFunc);
    as_function(valueFunc)->name = "value";
    function_set_empty_evaluation(as_function(valueFunc));

    // Initialize primitive types (this requires value() function)
    BOOL_TYPE = create_type_value(kernel, &BOOL_T, "bool");
    FLOAT_TYPE = create_type_value(kernel, &FLOAT_T, "number");
    INT_TYPE = create_type_value(kernel, &INT_T, "int");
    NAME_TYPE = create_type_value(kernel, &NAME_T, "Name");
    STRING_TYPE = create_type_value(kernel, &STRING_T, "String");
    DICT_TYPE = create_type_value(kernel, &DICT_T, "Dict");
    REF_TYPE = create_type_value(kernel, &REF_T, "Term");
    VOID_TYPE = create_type_value(kernel, &VOID_T, "void");
    TYPES.list = unbox_type(create_type_value(kernel, &LIST_T, "List"));
    OPAQUE_POINTER_TYPE = create_type_value(kernel, &OPAQUE_POINTER_T, "opaque_pointer");
    create_type_value(kernel, &BRANCH_T, "Branch");

    // Finish initializing World (this requires List type)
    world_initialize(g_world);

    // Setup output_placeholder() function, needed to declare functions properly.
    FUNCS.output = create_value(kernel, &FUNCTION_T, "output_placeholder");
    function_t::initialize(&FUNCTION_T, term_value(FUNCS.output));
    initialize_function(FUNCS.output);
    as_function(FUNCS.output)->name = "output_placeholder";
    as_function(FUNCS.output)->evaluate = NULL;
    as_function(FUNCS.output)->specializeType = output_placeholder_specializeType;
    ca_assert(function_get_output_type(FUNCS.output, 0) == &ANY_T);

    // Fix some holes in value() function
    {
        Function* attrs = as_function(valueFunc);
        Term* output = append_output_placeholder(function_contents(attrs), NULL);
        change_declared_type(output, &ANY_T);
        finish_building_function(function_contents(attrs));
    }

    ca_assert(function_get_output_type(valueFunc, 0) == &ANY_T);

    // input_placeholder() is needed before we can declare a function with inputs
    FUNCS.input = import_function(kernel, NULL, "input_placeholder() -> any");
    function_set_empty_evaluation(as_function(FUNCS.input));

    // Now that we have input_placeholder() let's declare one input on output_placeholder()
    apply(function_contents(as_function(FUNCS.output)),
        FUNCS.input, TermList())->setBoolProp("optional", true);

    namespace_function::early_setup(kernel);

    // Setup declare_field() function, needed to represent compound types.
    FUNCS.declare_field = import_function(kernel, NULL, "declare_field() -> any");

    FINISHED_BOOTSTRAP = true;

    // Set up some global constants
    set_bool(&TrueValue, true);
    set_bool(&FalseValue, false);

    // Initialize a few more types
    Term* set_type = create_value(kernel, &TYPE_T, "Set");
    set_t::setup_type(unbox_type(set_type));

    TYPES.map = unbox_type(create_value(kernel, &TYPE_T, "Map"));
    hashtable_t::setup_type(TYPES.map);

    Term* indexableType = create_value(kernel, &TYPE_T, "Indexable");
    indexable_t::setup_type(unbox_type(indexableType));

    TYPES.selector = unbox_type(create_value(kernel, &TYPE_T, "Selector"));
    list_t::setup_type(TYPES.selector);

    control_flow_setup_funcs(kernel);
    selector_setup_funcs(kernel);

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

    Term* mod_func = create_overloaded_function(kernel, "mod(any,any) -> any");
    append_to_overloaded_function(mod_func, kernel->get("mod_i"));
    append_to_overloaded_function(mod_func, kernel->get("mod_f"));

    FUNCS.mult = create_overloaded_function(kernel, "mult(any,any) -> any");
    append_to_overloaded_function(FUNCS.mult, kernel->get("mult_i"));
    append_to_overloaded_function(FUNCS.mult, kernel->get("mult_f"));

    FUNCS.neg = create_overloaded_function(kernel, "neg(any) -> any");
    append_to_overloaded_function(FUNCS.neg, kernel->get("neg_i"));
    append_to_overloaded_function(FUNCS.neg, kernel->get("neg_f"));
    as_function(FUNCS.neg)->formatSource = neg_function::formatSource;

    FUNCS.sub = create_overloaded_function(kernel, "sub(any,any) -> any");
    append_to_overloaded_function(FUNCS.sub, kernel->get("sub_i"));
    append_to_overloaded_function(FUNCS.sub, kernel->get("sub_f"));

    // Create vectorized functions
    Term* add_v = create_function(kernel, "add_v");
    create_function_vectorized_vv(function_contents(add_v), FUNCS.add, &LIST_T, &LIST_T);
    Term* add_s = create_function(kernel, "add_s");
    create_function_vectorized_vs(function_contents(add_s), FUNCS.add, &LIST_T, &ANY_T);

    append_to_overloaded_function(FUNCS.add, add_v);
    append_to_overloaded_function(FUNCS.add, add_s);

    Term* sub_v = create_function(kernel, "sub_v");
    create_function_vectorized_vv(function_contents(sub_v), FUNCS.sub, &LIST_T, &LIST_T);
    Term* sub_s = create_function(kernel, "sub_s");
    create_function_vectorized_vs(function_contents(sub_s), FUNCS.sub, &LIST_T, &ANY_T);
    
    append_to_overloaded_function(FUNCS.sub, sub_v);
    append_to_overloaded_function(FUNCS.sub, sub_s);

    // Create vectorized mult() functions
    Term* mult_v = create_function(kernel, "mult_v");
    create_function_vectorized_vv(function_contents(mult_v), FUNCS.mult, &LIST_T, &LIST_T);
    Term* mult_s = create_function(kernel, "mult_s");
    create_function_vectorized_vs(function_contents(mult_s), FUNCS.mult, &LIST_T, &ANY_T);

    append_to_overloaded_function(FUNCS.mult, mult_v);
    append_to_overloaded_function(FUNCS.mult, mult_s);

    Term* div_s = create_function(kernel, "div_s");
    create_function_vectorized_vs(function_contents(div_s), FUNCS.div, &LIST_T, &ANY_T);

    // Need dynamic_method before any hosted functions
    FUNCS.dynamic_method = import_function(kernel, dynamic_method_call,
            "def dynamic_method(any inputs :multiple) -> any");

    // Load the standard library from stdlib.ca
    parser::compile(kernel, parser::statement_list, STDLIB_CA_TEXT);

    // Install C functions
    static const ImportRecord records[] = {
        {"assert", hosted_assert},
        {"cppbuild:build_module", cppbuild_function::build_module},
        // {"dynamic_call", dynamic_call},
        {"file:version", file__version},
        {"file:exists", file__exists},
        {"file:read_text", file__read_text},
        {"length", length},
        {"from_string", from_string},
        {"to_string_repr", to_string_repr},
        {"call_actor", call_actor_func},
        {"send", send_func},
        {"test_spy", test_spy},
        {"test_oracle", test_oracle},
        {"refactor:rename", refactor__rename},
        {"refactor:change_function", refactor__change_function},
        {"reflect:this_branch", reflect__this_branch},
        {"reflect:kernel", reflect__kernel},
        {"sys:module_search_paths", sys__module_search_paths},
        {"sys:perf_stats_reset", sys__perf_stats_reset},
        {"sys:perf_stats_dump", sys__perf_stats_dump},
        {"load_module", load_module},

        // {"Branch.call", dynamic_call},

        {"Dict.count", Dict__count},
        {"Dict.get", Dict__get},
        {"Dict.set", Dict__set},

        {"Function.branch", Function__branch},

        {"List.append", List__append},
        {"List.extend", List__extend},
        {"List.resize", List__resize},
        {"List.count", List__count},
        {"List.insert", List__insert},
        {"List.length", List__length},
        {"List.join", List__join},
        {"List.slice", List__slice},
        {"List.get", List__get},

        {"Map.contains", Map__contains},
        {"Map.remove", Map__remove},
        {"Map.get", Map__get},
        {"Map.set", Map__set},

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
    reflection_install_functions(kernel);
    interpreter_install_functions(kernel);

    // Fetch refereneces to certain builtin funcs.
    FUNCS.dll_patch = kernel->get("sys:dll_patch");
    FUNCS.dynamic_call = kernel->get("dynamic_call");
    FUNCS.branch_dynamic_call = kernel->get("Branch.call");
    FUNCS.length = kernel->get("length");
    FUNCS.load_native_patch = kernel->get("load_native_patch");
    FUNCS.not_func = kernel->get("not");
    FUNCS.type = kernel->get("type");
    FUNCS.output_explicit = kernel->get("output");
    FUNCS.list_append = kernel->get("List.append");

    // Finish setting up some hosted types
    TYPES.actor = as_type(kernel->get("Actor"));
    TYPES.color = as_type(kernel->get("Color"));
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

EXPORT caWorld* circa_initialize()
{
    FINISHED_BOOTSTRAP = false;
    STATIC_INITIALIZATION_FINISHED = true;

    memset(&FUNCS, 0, sizeof(FUNCS));
    memset(&TYPES, 0, sizeof(TYPES));

    bootstrap_kernel();

    caWorld* world = create_world();

    Branch* kernel = global_root_branch();

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
            modules_add_search_path(as_cstring(path));
        }
    }

    log_msg(0, "finished circa_initialize");

    return world;
}

EXPORT void circa_shutdown(caWorld* world)
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

    delete world->root;
    world->root = NULL;

    memset(&FUNCS, 0, sizeof(FUNCS));

    name_dealloc_global_data();

    gc_collect();

    free(world);
}

} // namespace circa

// Public API

using namespace circa;

caBranch* circa_kernel(caWorld* world)
{
    return world->root;
}
