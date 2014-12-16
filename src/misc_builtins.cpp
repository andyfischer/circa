// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "closures.h"
#include "debug.h"
#include "file.h"
#include "hashtable.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "native_patch.h"
#include "rand.h"
#include "replication.h"
#include "stack.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"

#include "ext/perlin.h"

namespace circa {

void abs(caStack* stack)
{
    set_float(circa_output(stack, 0), std::abs(circa_float_input(stack, 0)));
}

void add_i_evaluate(caStack* stack)
{
    int sum = circa_int_input(stack, 0) + circa_int_input(stack, 1);
    set_int(circa_output(stack, 0), sum);
}

void add_f_evaluate(caStack* stack)
{
    float sum = circa_float_input(stack, 0) + circa_float_input(stack, 1);
    set_float(circa_output(stack, 0), sum);
}

void assert_func(caStack* stack)
{
    if (!circa_bool_input(stack, 0))
        circa_output_error(stack, "Assert failed");
}

void cond(caStack* stack)
{
    int index = circa_bool_input(stack, 0) ? 1 : 2;
    copy(circa_input(stack, index), circa_output(stack, 0));
}

void str(caStack* stack)
{
    Value* args = circa_input(stack, 0);
    Value* out = circa_output(stack, 0);
    set_string(out, "");

    for (int index=0; index < list_length(args); index++) {
        Value* v = circa_index(args, index);
        string_append(out, v);
    }
}

void copy_eval(caStack* stack)
{
    copy(circa_input(stack, 0), circa_output(stack, 0));
}

void cast_declared_type(caStack* stack)
{
    Value* source = circa_input(stack, 0);

    Type* type = circa_caller_term(stack)->type;

    if (type == TYPES.any)
        return move(source, circa_output(stack, 0));

    if (!cast_possible(source, type)) {
        std::stringstream message;
        Value msg;
        string_append(&msg, "Can't cast value ");
        string_append_quoted(&msg, source);
        string_append(&msg, " to type ");
        string_append(&msg, &type->name);

        return circa_output_error(stack, as_cstring(&msg));
    }

    Value* output = circa_output(stack, 0);
    move(source, output);
    touch(output);
    bool success = cast(output, type);

    if (!success)
        return circa_output_error(stack, "cast failed");
}

void cast_evaluate(caStack* stack)
{
    // 'cast' is used by scripts. In this version, the type is passed via the 2nd param
    Value* result = circa_output(stack, 0);
    move(circa_input(stack, 0), result);

    Type* type = unbox_type(circa_input(stack, 1));

    bool success = cast(result, type);
    set_bool(circa_output(stack, 1), success);
}

void debug_break(Stack* stack)
{
    printf("debug_break..\n");
}

void div_f(caStack* stack)
{
    set_float(circa_output(stack, 0), circa_float_input(stack, 0) / circa_float_input(stack, 1));
}

void div_i(caStack* stack)
{
    int a = to_int(circa_input(stack, 0));
    int b = to_int(circa_input(stack, 1));
    set_int(circa_output(stack, 0), a / b);
}

void empty_list(caStack* stack)
{
    Value* out = circa_output(stack, 0);
    int size = circa_int_input(stack, 1);
    Value* initialValue = circa_input(stack, 0);
    set_list(out, size);
    for (int i=0; i < size; i++) {
        copy(initialValue, list_get(out, i));
    }
}

void equals_func(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            equals(circa_input(stack, 0), circa_input(stack, 1)));
}

void hosted_is_compound(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_struct(circa_input(stack, 0)));
}

void hosted_is_list(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_list(circa_input(stack, 0)));
}
void hosted_is_int(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_int(circa_input(stack, 0)));
}
void hosted_is_map(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_hashtable(circa_input(stack, 0)));
}
void hosted_is_number(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_float(circa_input(stack, 0)));
}
void hosted_is_bool(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_bool(circa_input(stack, 0)));
}
void hosted_is_string(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_string(circa_input(stack, 0)));
}
void hosted_is_null(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_null(circa_input(stack, 0)));
}
void hosted_is_function(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_func(circa_input(stack, 0)));
}
void hosted_is_type(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_type(circa_input(stack, 0)));
}

void inputs_fit_function(caStack* stack)
{
    Value* inputs = circa_input(stack, 0);
    Term* function = circa_caller_input_term(stack, 1);
    Block* functionContents = nested_contents(function);
    Value* result = circa_output(stack, 0);

    bool varArgs = has_variable_args(functionContents);

    // Fail if wrong # of inputs
    if (!varArgs && (count_input_placeholders(functionContents) != circa_count(inputs))) {
        set_bool(result, false);
        return;
    }

    // Check each input
    for (int i=0; i < circa_count(inputs); i++) {
        Term* placeholder = get_effective_input_placeholder(functionContents, i);
        Value* value = circa_index(inputs, i);
        if (value == NULL)
            continue;
        if (!cast_possible(value, declared_type(placeholder))) {
            set_bool(result, false);
            return;
        }
    }

    set_bool(result, true);
}
void overload_error_no_match(caStack* stack)
{
    Value* inputs = circa_input(stack, 00);

    Term* caller = (Term*) circa_caller_term(stack);
    Term* func = parent_term(caller, 3);

    std::stringstream out;
    Value msg;
    string_append(&msg, "In overloaded function ");

    if (func == NULL)
        string_append(&msg, "<name not found>");
    else
        string_append(&msg, term_name(func));
    string_append(&msg, ", no func could handle inputs: ");
    string_append(&msg, inputs);
    circa_output_error(stack, as_cstring(&msg));
}

void unique_id(caStack* stack)
{
    static int nextId = 1; // TODO: this should probably be stored on World?
    set_int(circa_output(stack, 0), nextId++);
}

void source_id(Stack* stack)
{
#if 0
    // TODO
    Term* caller = frame_current_term(top_frame_parent(stack));
    get_global_name(caller, circa_output(stack, 0));
#endif
}

void unknown_function(caStack* stack)
{
    std::string out;
    out += "Unknown function: ";
    Term* caller = (Term*) circa_caller_term(stack);
    out += caller->stringProp(sym_Syntax_FunctionName, "");
    circa_output_error(stack, out.c_str());
}

void unknown_identifier(caStack* stack)
{
    Value msg;
    set_string(&msg, "Unknown identifier: ");
    string_append(&msg, term_name(circa_caller_term(stack)));
    circa_output_error(stack, as_cstring(&msg));
}

void write_text_file_func(caStack* stack)
{
    write_text_file(circa_string_input(stack, 0), circa_string_input(stack, 1));
}

void get_field(caStack* stack)
{
    Value* head = circa_input(stack, 0);

    Value error;
    Value* value = get_field(head, circa_input(stack, 1), &error);

    if (!is_null(&error)) {
        circa_output_error(stack, as_cstring(&error));
        return;
    }

    ca_assert(value != NULL);

    copy(value, circa_output(stack, 0));
}

void get_index(caStack* stack)
{
    Value* list = circa_input(stack, 0);
    int index = circa_int_input(stack, 1);

    if (index < 0) {
        char indexStr[40];
        sprintf(indexStr, "Negative index: %d", index);
        return circa_output_error(stack, indexStr);
    } else if (index >= list_length(list)) {
        char indexStr[40];
        sprintf(indexStr, "Index out of range: %d", index);
        return circa_output_error(stack, indexStr);
    }

    Value* result = get_index(list, index);

    copy(result, circa_output(stack, 0));
    cast(circa_output(stack, 0), declared_type((Term*) circa_caller_term(stack)));
}

void make_list(caStack* stack)
{
    // Variadic arg handling will already have turned this into a list
    Value* out = circa_output(stack, 0);
    move(circa_input(stack, 0), out);
    if (!circa_is_list(out))
        circa_set_list(out, 0);
}

void make_map(Stack* stack)
{
    Value* out = circa_output(stack, 0);
    Value* args = circa_input(stack, 0);
    int len = list_length(args);
    if ((len % 2) != 0)
        return circa_output_error(stack, "Number of arguments must be an even number");

    set_hashtable(out);
    for (int i=0; i < len; i += 2) {
        Value* key = list_get(args, i);
        Value* val = list_get(args, i+1);
        move(val, hashtable_insert(out, key, true));
    }
}

void blank_list(caStack* stack)
{
    Value* out = circa_output(stack, 0);
    int count = circa_int_input(stack, 0);
    circa_set_list(out, count);
}

void and_func(caStack* stack)
{
    set_bool(circa_output(stack, 0),
        circa_bool_input(stack, 0) && circa_bool_input(stack, 1));
}

void or_func(caStack* stack)
{
    set_bool(circa_output(stack, 0),
        circa_bool_input(stack, 0) || circa_bool_input(stack, 1));
}

void not_func(caStack* stack)
{
    set_bool(circa_output(stack, 0), !circa_bool_input(stack, 0));
}

void make_func(caStack* stack)
{
    make(as_type(circa_input(stack, 0)), circa_output(stack, 0));
}

void max_f(caStack* stack)
{
    set_float(circa_output(stack, 0),
            std::max(circa_float_input(stack, 0), circa_float_input(stack, 1)));
}

void max_i(caStack* stack)
{
    set_int(circa_output(stack, 0),
            std::max(circa_int_input(stack, 0), circa_int_input(stack, 1)));
}

void min_f(caStack* stack)
{
    set_float(circa_output(stack, 0),
            std::min(circa_float_input(stack, 0), circa_float_input(stack, 1)));
}

void min_i(caStack* stack)
{
    set_int(circa_output(stack, 0),
            std::min(circa_int_input(stack, 0), circa_int_input(stack, 1)));
}

void remainder_i(caStack* stack)
{
    set_int(circa_output(stack, 0), circa_int_input(stack, 0) % circa_int_input(stack, 1));
}

void remainder_f(caStack* stack)
{
    set_float(circa_output(stack, 0), fmodf(circa_float_input(stack, 0), circa_float_input(stack, 1)));
}

// We compute mod() using floored division. This is different than C and many
// C-like languages which use truncated division. See this page for an explanation
// of the difference:
// http://en.wikipedia.org/wiki/Modulo_operation
//
// For a function that works the same as C's modulo, use remainder() . The % operator
// also uses remainder(), so that it works the same as C's % operator.

void mod_i(caStack* stack)
{
    int a = circa_int_input(stack, 0);
    int n = circa_int_input(stack, 1);

    int out = a % n;
    if (out < 0)
        out += n;

    set_int(circa_output(stack, 0), out);
}

void mod_f(caStack* stack)
{
    float a = circa_float_input(stack, 0);
    float n = circa_float_input(stack, 1);

    float out = fmodf(a, n);

    if (out < 0)
        out += n;

    set_float(circa_output(stack, 0), out);
}

void round(caStack* stack)
{
    float input = circa_float_input(stack, 0);
    if (input > 0.0)
        set_int(circa_output(stack, 0), int(input + 0.5));
    else
        set_int(circa_output(stack, 0), int(input - 0.5));
}

void floor(caStack* stack)
{
    set_int(circa_output(stack, 0), (int) std::floor(circa_float_input(stack, 0)));
}

void ceil(caStack* stack)
{
    set_int(circa_output(stack, 0), (int) std::ceil(circa_float_input(stack, 0)));
}

void average(caStack* stack)
{
    Value* args = circa_input(stack, 0);
    int count = circa_count(args);
    Value* out = circa_output(stack, 0);

    if (count == 0) {
        set_float(out, 0);
        return;
    }

    float sum = 0;
    for (int i=0; i < count; i++)
        sum += to_float(circa_index(args, i));

    set_float(out, sum / count);
}

void pow(caStack* stack)
{
    set_float(circa_output(stack, 0),
            std::pow((float) to_float(circa_input(stack, 0)), to_float(circa_input(stack, 1))));
}

void sqr(caStack* stack)
{
    float in = circa_float_input(stack, 0);
    set_float(circa_output(stack, 0), in * in);
}
void cube(caStack* stack)
{
    float in = circa_float_input(stack, 0);
    set_float(circa_output(stack, 0), in * in * in);
}

void sqrt(caStack* stack)
{
    set_float(circa_output(stack, 0), std::sqrt(circa_float_input(stack, 0)));
}

void log(caStack* stack)
{
    set_float(circa_output(stack, 0), std::log(circa_float_input(stack, 0)));
}


void mult_f(caStack* stack)
{
    float product = circa_float_input(stack, 0) * circa_float_input(stack, 1);
    set_float(circa_output(stack, 0), product);
}

void mult_i(caStack* stack)
{
    int product = circa_int_input(stack, 0) * circa_int_input(stack, 1);
    set_int(circa_output(stack, 0), product);
}

void neg_f(caStack* stack)
{
    set_float(circa_output(stack, 0), -circa_float_input(stack, 0));
}

void neg_i(caStack* stack)
{
    set_int(circa_output(stack, 0), -circa_int_input(stack, 0));
}

void sub_i(caStack* stack)
{
    set_int(circa_output(stack, 0), circa_int_input(stack, 0) - circa_int_input(stack, 1));
}

void sub_f(caStack* stack)
{
    set_float(circa_output(stack, 0), circa_float_input(stack, 0) - circa_float_input(stack, 1));
}

float radians_to_degrees(float radians) { return radians * 180.0f / M_PI; }
float degrees_to_radians(float unit) { return unit * M_PI / 180.0f; }

void sin_func(caStack* stack)
{
    float input = circa_float_input(stack, 0);

    set_float(circa_output(stack, 0), sin(degrees_to_radians(input)));
}
void cos_func(caStack* stack)
{
    float input = circa_float_input(stack, 0);

    set_float(circa_output(stack, 0), cos(degrees_to_radians(input)));
}
void tan_func(caStack* stack)
{
    float input = circa_float_input(stack, 0);

    set_float(circa_output(stack, 0), tan(degrees_to_radians(input)));
}
void arcsin_func(caStack* stack)
{
    float result = asin(circa_float_input(stack, 0));
    set_float(circa_output(stack, 0), radians_to_degrees(result));
}
void arccos_func(caStack* stack)
{
    float result = acos(circa_float_input(stack, 0));
    set_float(circa_output(stack, 0), radians_to_degrees(result));
}
void arctan_func(caStack* stack)
{
    float result = atan(circa_float_input(stack, 0));
    set_float(circa_output(stack, 0), radians_to_degrees(result));
}

void range(caStack* stack)
{
    int start = circa_int_input(stack, 0);
    int max = circa_int_input(stack, 1);

    unsigned count = std::abs(max - start);
    
    if (count > 1000000)
        return circa_output_error(stack, "Range is too large");
    
    Value* output = circa_output(stack, 0);
    set_list(output, count);

    int val = start;
    int increment = start < max ? 1 : -1;
    for (int i=0; i < count; i++) {
        set_int(list_get(output, i), val);
        val += increment;
    }
}

void rpath(caStack* stack)
{
    caBlock* block = circa_caller_block(stack);
    get_path_relative_to_source(block, circa_input(stack, 0), circa_output(stack, 0));
}

void set_field(caStack* stack)
{
    stat_increment(SetField);

    Value* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);
    touch(out);

    Value* name = circa_input(stack, 1);

    Value* slot = get_field(out, name, NULL);
    if (slot == NULL) {
        circa::Value msg;
        set_string(&msg, "field not found: ");
        string_append(&msg, name);
        return raise_error_msg(stack, as_cstring(&msg));
    }

    copy(circa_input(stack, 2), slot);
}

void set_index(caStack* stack)
{
    stat_increment(SetIndex);

    Value* output = circa_output(stack, 0);
    copy(circa_input(stack, 0), output);
    touch(output);
    int index = circa_int_input(stack, 1);
    copy(circa_input(stack, 2), list_get(output, index));
}
void rand(caStack* stack)
{
    set_float(circa_output(stack, 0), rand_next_double(&stack->randState));
}

void repeat(caStack* stack)
{
    Value* source = circa_input(stack, 0);
    int repeatCount = circa_int_input(stack, 1);

    Value* out = circa_output(stack, 0);
    circa_set_list(out, repeatCount);

    for (int i=0; i < repeatCount; i++)
        copy(source, circa_index(out, i));
}

void int__to_hex_string(caStack* stack)
{
    std::stringstream strm;
    strm << std::hex << as_int(circa_input(stack, 0));
    set_string(circa_output(stack, 0), strm.str().c_str());
}

void less_than_i(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            circa_int_input(stack, 0) < circa_int_input(stack, 1));
}

void less_than_f(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            circa_float_input(stack, 0) < circa_float_input(stack, 1));
}

void less_than_eq_i(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            circa_int_input(stack, 0) <= circa_int_input(stack, 1));
}

void less_than_eq_f(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            circa_float_input(stack, 0) <= circa_float_input(stack, 1));
}

void greater_than_i(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            circa_int_input(stack, 0) > circa_int_input(stack, 1));
}

void greater_than_f(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            circa_float_input(stack, 0) > circa_float_input(stack, 1));
}

void greater_than_eq_i(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            circa_int_input(stack, 0) >= circa_int_input(stack, 1));
}

void greater_than_eq_f(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            circa_float_input(stack, 0) >= circa_float_input(stack, 1));
}

void List__append(caStack* stack)
{
    Value* out = circa_output(stack, 0);
    move(circa_input(stack, 0), out);
    move(circa_input(stack, 1), list_append(out));
}

void List__concat(caStack* stack)
{
    Value* out = circa_output(stack, 0);
    move(circa_input(stack, 0), out);

    Value* additions = circa_input(stack, 1);
    touch(additions);

    int oldLength = list_length(out);
    int additionsLength = list_length(additions);

    list_resize(out, oldLength + additionsLength);
    for (int i = 0; i < additionsLength; i++)
        move(list_get(additions, i), list_get(out, oldLength + i));
}

void List__resize(caStack* stack)
{
    Value* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);
    int count = circa_int_input(stack, 1);
    circa_resize(out, count);
}

void List__extend(caStack* stack)
{
    Value* out = circa_output(stack, 1);
    copy(circa_input(stack, 0), out);

    Value* additions = circa_input(stack, 1);

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
    Value* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);

    copy(circa_input(stack, 2), list_insert(out, circa_int_input(stack, 1)));
}

void List__slice(caStack* stack)
{
    Value* input = circa_input(stack, 0);
    int start = circa_int_input(stack, 1);
    int end = circa_int_input(stack, 2);
    Value* output = circa_output(stack, 0);

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
    Value* input = circa_input(stack, 0);
    Value* joiner = circa_input(stack, 1);

    Value* out = circa_output(stack, 0);
    set_string(out, "");

    for (int i=0; i < list_length(input); i++) {
        if (i != 0)
            string_append(out, joiner);

        string_append(out, list_get(input, i));
    }
}

void List__get(caStack* stack)
{
    Value* self = circa_input(stack, 0);
    int index = circa_int_input(stack, 1);
    if (index < 0 || index >= list_length(self))
        return raise_error_msg(stack, "Index out of bounds");

    copy(list_get(self, index), circa_output(stack, 0));
}

void List__set(caStack* stack)
{
    Value* self = circa_output(stack, 0);
    move(circa_input(stack, 0), self);
    touch(self);

    int index = circa_int_input(stack, 1);
    Value* value = circa_input(stack, 2);

    move(value, list_get(self, index));
}

void List__remove(caStack* stack)
{
    Value* self = circa_output(stack, 0);
    move(circa_input(stack, 0), self);
    int index = circa_int_input(stack, 1);

    if (index < 0 || index >= list_length(self))
        return circa_output_error(stack, "Invalid index");

    list_remove_index(self, index);
}

void Map__contains(caStack* stack)
{
    Value* key = circa_input(stack, 1);
    if (!value_hashable(key))
        return circa_output_error(stack, "Key is not hashable");

    Value* value = hashtable_get(circa_input(stack, 0), key);
    set_bool(circa_output(stack, 0), value != NULL);
}

void Map__keys(caStack* stack)
{
    Value* table = circa_input(stack, 0);
    hashtable_get_keys(table, circa_output(stack, 0));
}

void Map__remove(caStack* stack)
{
    Value* key = circa_input(stack, 1);
    if (!value_hashable(key))
        return circa_output_error(stack, "Key is not hashable");

    Value* self = circa_output(stack, 0);
    move(circa_input(stack, 0), self);
    hashtable_remove(self, key);
}

void Map__get(caStack* stack)
{
    Value* table = circa_input(stack, 0);
    Value* key = circa_input(stack, 1);
    if (!value_hashable(key))
        return circa_output_error(stack, "Key is not hashable");

    Value* value = hashtable_get(table, key);
    if (value == NULL) {
        Value msg;
        set_string(&msg, "Key not found: ");
        string_append_quoted(&msg, key);
        return circa_output_error(stack, as_cstring(&msg));
    }
    copy(value, circa_output(stack, 0));
}

void Map__set(caStack* stack)
{
    Value* key = circa_input(stack, 1);
    if (!value_hashable(key))
        return circa_output_error(stack, "Key is not hashable");

    Value* self = circa_output(stack, 0);
    move(circa_input(stack, 0), self);

    Value* value = circa_input(stack, 2);
    move(value, hashtable_insert(self, key, false));
}

void Map__empty(caStack* stack)
{
    set_bool(circa_output(stack, 0), hashtable_is_empty(circa_input(stack, 0)));
}

void Module__block(caStack* stack)
{
    Value* moduleRef = circa_input(stack, 0);
    Block* moduleBlock = module_ref_resolve(stack->world, moduleRef);
    set_block(circa_output(stack, 0), moduleBlock);
}

void Module__get(caStack* stack)
{
    Value* moduleRef = circa_input(stack, 0);
    Block* moduleBlock = module_ref_resolve(stack->world, moduleRef);
    Term* term = find_local_name(moduleBlock, circa_input(stack, 1));
    copy(term_value(term), circa_output(stack, 0));
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

void String__char_code(caStack* stack)
{
    const char* str = circa_string_input(stack, 0);
    if (strlen(str) != 1)
        return circa_output_error(stack, "Expected a string of length 1");
    
    set_int(circa_output(stack, 0), (int) str[0]);
}

void String__from_char_code(Stack* stack)
{
    char str[2];
    str[0] = circa_int_input(stack, 1);
    str[1] = 0;
    set_string(circa_output(stack, 0), str);
}

void String__substr(caStack* stack)
{
    Value* self = circa_input(stack, 0);
    int start = circa_int_input(stack, 1);
    int length = circa_int_input(stack, 2);

    if (start < 0)
        start = 0;

    int existingLength = string_length(self);

    if (length == -1)
        length = existingLength - start;

    if (start + length > existingLength)
        length = existingLength - start;

    if (length < 0)
        length = 0;

    if (start > existingLength)
        start = existingLength;

    set_string(circa_output(stack, 0), as_cstring(self) + start, length);
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

void String__to_number(caStack* stack)
{
    float n = atof(circa_string_input(stack, 0));
    set_float(circa_output(stack, 0), n);
}

void String__to_int(caStack* stack)
{
    int n = atoi(circa_string_input(stack, 0));
    set_int(circa_output(stack, 0), n);
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

Value* find_env_value(caStack* stack, Value* key)
{
    stat_increment(FindEnvValue);

    for (Frame* frame = top_frame(stack); frame != NULL; frame = prev_frame(frame)) {
        if (!is_null(&frame->env)) {
            Value* value = hashtable_get(&frame->env, key);
            if (value != NULL)
                return value;
        }
    }

    if (!is_null(&stack->env)) {
        Value* value = hashtable_get(&stack->env, key);
        if (value != NULL)
            return value;
    }

    if (stack->caller != NULL)
        // Keep searching up to the calling stack
        return find_env_value(stack->caller, key);

    return NULL;
}

void get_env(caStack* stack)
{
    Value* value = find_env_value(stack, circa_input(stack, 0));
    if (value != NULL)
        copy(value, circa_output(stack, 0));
    else
        set_null(circa_output(stack, 0));
}

void get_env_opt(caStack* stack)
{
    Value* value = find_env_value(stack, circa_input(stack, 0));
    if (value != NULL)
        copy(value, circa_output(stack, 0));
    else
        copy(circa_input(stack, 1), circa_output(stack, 0));
}

void set_env(caStack* stack)
{
    Value* key = circa_input(stack, 0);
    Value* value = circa_input(stack, 1);

    copy(value, stack_env_insert(stack, key));
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

void channel_send(caStack* stack)
{
    Value* name = circa_input(stack, 0);
    Value* channel = find_env_value(stack, name);
    if (channel == NULL) {
        channel = stack_env_insert(stack, name);
        set_list(channel, 0);
    }

    move(circa_input(stack, 1), list_append(channel));
}

void channel_read(caStack* stack)
{
    Value* name = circa_input(stack, 0);
    Value* channel = find_env_value(stack, name);
    if (channel == NULL)
        set_list(circa_output(stack, 0), 0);
    else {
        move(channel, circa_output(stack, 0));
        set_list(channel, 0);
    }
}

void find_active_value(caStack* stack)
{
    Term* term = as_term_ref(circa_input(stack, 0));
    Value* value = stack_find_nonlocal(top_frame(stack), term);

    if (value == NULL) {
        set_bool(circa_output(stack, 1), false);
    } else {
        set_value(circa_output(stack, 0), value);
        set_bool(circa_output(stack, 1), true);
    }
}

void typeof_func(caStack* stack)
{
    Value* in = circa_input(stack, 0);
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

void noise(Stack* stack)
{
    const int octaves = 4;
    float out = perlin_fbm(octaves, circa_float_input(stack, 0));
    set_float(circa_output(stack, 0), out);
}

void not_equals(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            !equals(circa_input(stack, 0), circa_input(stack, 1)));
}

void error(caStack* stack)
{
    Value* args = circa_input(stack, 0);

    Value out;

    for (int i = 0; i < circa_count(args); i++) {
        Value* val = circa_index(args, i);
        string_append(&out, val);
    }

    circa_output_error_val(stack, &out);
}

void get_with_symbol(Stack* stack)
{
    Value* left = circa_input(stack, 0);
    Value str;
    symbol_as_string(circa_input(stack, 1), &str);

    if (is_module_ref(left)) {
        Block* block = module_ref_resolve(stack->world, left);
        Term* term = find_local_name(block, &str);

        if (term != NULL) {
            set_closure(circa_output(stack, 0), term->nestedContents, NULL);
            return;
        }
    }

    circa_output_error(stack, "Symbol not found");
}

void print(caStack* stack)
{
    Value* args = circa_input(stack, 0);

    Value out;
    set_string(&out, "");

    for (int i = 0; i < circa_count(args); i++) {
        Value* val = circa_index(args, i);
        string_append(&out, val);
    }

    write_log(as_cstring(&out));

    set_null(circa_output(stack, 0));
}

void to_string(caStack* stack)
{
    Value* in = circa_input(stack, 0);
    Value* out = circa_output(stack, 0);
    to_string(in, out);
}

void compute_patch_hosted(caStack* stack)
{
    Value error;

    compute_value_patch(circa_input(stack, 0), circa_input(stack, 1),
        circa_output(stack, 0), &error);

    if (!is_null(&error))
        circa_output_error_val(stack, &error);
}

void apply_patch_hosted(caStack* stack)
{
    Value* result = circa_output(stack, 0);
    copy(circa_input(stack, 0), result);
    apply_patch(result, circa_input(stack, 1));
}

void misc_builtins_setup_functions(NativePatch* patch)
{
    circa_patch_function(patch, "add_i", add_i_evaluate);
    circa_patch_function(patch, "add_f", add_f_evaluate);
    circa_patch_function(patch, "abs", abs);
    circa_patch_function(patch, "assert", assert_func);
    circa_patch_function(patch, "cast_declared_type", cast_declared_type);
    circa_patch_function(patch, "cast", cast_evaluate);
    circa_patch_function(patch, "debug_break", debug_break);
    circa_patch_function(patch, "str", str);
    circa_patch_function(patch, "cond", cond);
    circa_patch_function(patch, "copy", copy_eval);
    circa_patch_function(patch, "div_f", div_f);
    circa_patch_function(patch, "div_i", div_i);
    circa_patch_function(patch, "empty_list", empty_list);
    circa_patch_function(patch, "equals", equals_func);
    circa_patch_function(patch, "error", error);
    circa_patch_function(patch, "get_field", get_field);
    circa_patch_function(patch, "get_index", get_index);
    circa_patch_function(patch, "get_with_symbol", get_with_symbol);
    circa_patch_function(patch, "is_compound", hosted_is_compound);
    circa_patch_function(patch, "is_list", hosted_is_list);
    circa_patch_function(patch, "is_int", hosted_is_int);
    circa_patch_function(patch, "is_map", hosted_is_map);
    circa_patch_function(patch, "is_number", hosted_is_number);
    circa_patch_function(patch, "is_bool", hosted_is_bool);
    circa_patch_function(patch, "is_string", hosted_is_string);
    circa_patch_function(patch, "is_null", hosted_is_null);
    circa_patch_function(patch, "is_function", hosted_is_function);
    circa_patch_function(patch, "is_type", hosted_is_type);
    circa_patch_function(patch, "length", length);
    circa_patch_function(patch, "int.to_hex_string", int__to_hex_string);
    circa_patch_function(patch, "less_than_i", less_than_i);
    circa_patch_function(patch, "less_than_f", less_than_f);
    circa_patch_function(patch, "less_than_eq_i", less_than_eq_i);
    circa_patch_function(patch, "less_than_eq_f", less_than_eq_f);
    circa_patch_function(patch, "greater_than_i", greater_than_i);
    circa_patch_function(patch, "greater_than_f", greater_than_f);
    circa_patch_function(patch, "greater_than_eq_i", greater_than_eq_i);
    circa_patch_function(patch, "greater_than_eq_f", greater_than_eq_f);
    circa_patch_function(patch, "list", make_list);
    circa_patch_function(patch, "blank_list", blank_list);
    circa_patch_function(patch, "map", make_map);
    circa_patch_function(patch, "and", and_func);
    circa_patch_function(patch, "or", or_func);
    circa_patch_function(patch, "not", not_func);
    circa_patch_function(patch, "make", make_func);
    circa_patch_function(patch, "max_f", max_f);
    circa_patch_function(patch, "max_i", max_i);
    circa_patch_function(patch, "min_f", min_f);
    circa_patch_function(patch, "min_i", min_i);
    circa_patch_function(patch, "mod_i", mod_i);
    circa_patch_function(patch, "mod_f", mod_f);
    circa_patch_function(patch, "mult_i", mult_i);
    circa_patch_function(patch, "mult_f", mult_f);
    circa_patch_function(patch, "neg_i", neg_i);
    circa_patch_function(patch, "neg_f", neg_f);
    circa_patch_function(patch, "remainder_i", remainder_i);
    circa_patch_function(patch, "remainder_f", remainder_f);
    circa_patch_function(patch, "round", round);
    circa_patch_function(patch, "sub_i", sub_i);
    circa_patch_function(patch, "sub_f", sub_f);
    circa_patch_function(patch, "floor", floor);
    circa_patch_function(patch, "ceil", ceil);
    circa_patch_function(patch, "average", average);
    circa_patch_function(patch, "pow", pow);
    circa_patch_function(patch, "sqr", sqr);
    circa_patch_function(patch, "cube", cube);
    circa_patch_function(patch, "sqrt", sqrt);
    circa_patch_function(patch, "log", log);

    circa_patch_function(patch, "sin", sin_func);
    circa_patch_function(patch, "cos", cos_func);
    circa_patch_function(patch, "tan", tan_func);
    circa_patch_function(patch, "arcsin", arcsin_func);
    circa_patch_function(patch, "arccos", arccos_func);
    circa_patch_function(patch, "arctan", arctan_func);

    circa_patch_function(patch, "range", range);
    circa_patch_function(patch, "rpath", rpath);
    circa_patch_function(patch, "set_field", set_field);
    circa_patch_function(patch, "set_index", set_index);
    
    circa_patch_function(patch, "List.append", List__append);
    circa_patch_function(patch, "List.concat", List__concat);
    circa_patch_function(patch, "List.resize", List__resize);
    circa_patch_function(patch, "List.count", List__count);
    circa_patch_function(patch, "List.insert", List__insert);
    circa_patch_function(patch, "List.length", List__length);
    circa_patch_function(patch, "List.join", List__join);
    circa_patch_function(patch, "List.slice", List__slice);
    circa_patch_function(patch, "List.get", List__get);
    circa_patch_function(patch, "List.set", List__set);
    circa_patch_function(patch, "List.remove", List__remove);
    circa_patch_function(patch, "Map.contains", Map__contains);
    circa_patch_function(patch, "Map.keys", Map__keys);
    circa_patch_function(patch, "Map.remove", Map__remove);
    circa_patch_function(patch, "Map.get", Map__get);
    circa_patch_function(patch, "Map.set", Map__set);
    circa_patch_function(patch, "Map.empty", Map__empty);
    circa_patch_function(patch, "Module.block", Module__block);
    circa_patch_function(patch, "Module._get", Module__get);

    circa_patch_function(patch, "String.char_at", String__char_at);
    circa_patch_function(patch, "String.ends_with", String__ends_with);
    circa_patch_function(patch, "String.length", String__length);
    circa_patch_function(patch, "String.char_code", String__char_code);
    circa_patch_function(patch, "String.from_char_code", String__from_char_code);
    circa_patch_function(patch, "String.substr", String__substr);
    circa_patch_function(patch, "String.slice", String__slice);
    circa_patch_function(patch, "String.starts_with", String__starts_with);
    circa_patch_function(patch, "String.split", String__split);
    circa_patch_function(patch, "String.to_camel_case", String__to_camel_case);
    circa_patch_function(patch, "String.to_upper", String__to_upper);
    circa_patch_function(patch, "String.to_lower", String__to_lower);
    circa_patch_function(patch, "String.to_number", String__to_number);
    circa_patch_function(patch, "String.to_int", String__to_int);

    circa_patch_function(patch, "env", get_env);
    circa_patch_function(patch, "env_opt", get_env_opt);
    circa_patch_function(patch, "set_env", set_env);

    circa_patch_function(patch, "file_version", file__version);
    circa_patch_function(patch, "file_exists", file__exists);
    circa_patch_function(patch, "file_read_text", file__read_text);
    circa_patch_function(patch, "channel_send", channel_send);
    circa_patch_function(patch, "channel_read", channel_read);
    circa_patch_function(patch, "_find_active_value", find_active_value);

    circa_patch_function(patch, "noise", noise);
    circa_patch_function(patch, "not_equals", not_equals);
    circa_patch_function(patch, "print", print);
    circa_patch_function(patch, "rand", rand);
    circa_patch_function(patch, "repeat", repeat);
    circa_patch_function(patch, "to_string", to_string);
    circa_patch_function(patch, "trace", print);
    circa_patch_function(patch, "type", typeof_func);
    circa_patch_function(patch, "static_type", static_type_func);
    circa_patch_function(patch, "compute_patch", compute_patch_hosted);
    circa_patch_function(patch, "apply_patch", apply_patch_hosted);
    circa_patch_function(patch, "inputs_fit_function", inputs_fit_function);
    circa_patch_function(patch, "overload_error_no_match", overload_error_no_match);
    circa_patch_function(patch, "unique_id", unique_id);
    circa_patch_function(patch, "source_id", source_id);
    circa_patch_function(patch, "unknown_identifier", unknown_identifier);
    circa_patch_function(patch, "unknown_function", unknown_function);
    circa_patch_function(patch, "write_text_file", write_text_file_func);
}

} // namespace circa
