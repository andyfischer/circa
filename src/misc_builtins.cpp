// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "closures.h"
#include "debug.h"
#include "file.h"
#include "hashtable.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "native_patch.h"
#include "rand.h"
#include "replication.h"
#include "string_repr.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "vm.h"
#include "world.h"

#include "ext/perlin.h"

namespace circa {

Value* g_oracleValues;
Value* g_spyValues;

void abs(VM* vm)
{
    set_float(circa_output(vm), std::abs(vm->input(0)->to_f()));
}

void add_i(VM* vm)
{
    int sum = vm->input(0)->as_i() + vm->input(1)->as_i();
    vm->output()->set_int(sum);
}

void add_f(VM* vm)
{
    float sum = vm->input(0)->to_f() + vm->input(1)->to_f();
    vm->output()->set_float(sum);
}

void assert_func(VM* vm)
{
    if (!vm->input(0)->asBool())
        circa_throw(vm, "Assert failed");
}

void cond(VM* vm)
{
    int index = vm->input(0)->asBool() ? 1 : 2;
    move(vm->input(index), vm->output());
}

void str(VM* vm)
{
    Value args;
    move(circa_input(vm, 0), &args);

    Value* out = circa_output(vm);
    set_string(out, "");

    for (int index=0; index < list_length(&args); index++) {
        Value* v = circa_index(&args, index);
        string_append(out, v);
    }
}

void copy_eval(VM* vm)
{
    copy(vm->input(0), vm->output());
}

void cast_evaluate(VM* vm)
{
    // Another version of cast(), this one returns a nullable output.
    Value* result = circa_output(vm);
    move(circa_input(vm, 0), result);

    Type* type = unbox_type(circa_input(vm, 1));

    bool success = cast(result, type);
    if (!success)
        set_null(result);
}

void div_f(VM* vm)
{
    set_float(circa_output(vm), circa_input(vm, 0)->as_f() / circa_input(vm, 1)->as_f());
}

void div_i(VM* vm)
{
    int a = to_int(circa_input(vm, 0));
    int b = to_int(circa_input(vm, 1));
    set_int(circa_output(vm), a / b);
}

void empty_list(VM* vm)
{
    Value* out = circa_output(vm);
    int size = circa_input(vm, 1)->as_i();
    Value* initialValue = circa_input(vm, 0);
    set_list(out, size);
    for (int i=0; i < size; i++) {
        copy(initialValue, list_get(out, i));
    }
}

void equals_func(VM* vm)
{
    set_bool(circa_output(vm),
            equals(circa_input(vm, 0), circa_input(vm, 1)));
}

void hosted_is_compound(VM* vm)
{
    set_bool(circa_output(vm), is_struct(circa_input(vm, 0)));
}

void hosted_is_list(VM* vm)
{
    set_bool(circa_output(vm), is_list(circa_input(vm, 0)));
}
void hosted_is_int(VM* vm)
{
    set_bool(circa_output(vm), is_int(circa_input(vm, 0)));
}
void hosted_is_table(VM* vm)
{
    set_bool(circa_output(vm), is_hashtable(circa_input(vm, 0)));
}
void hosted_is_number(VM* vm)
{
    set_bool(circa_output(vm), is_float(circa_input(vm, 0)));
}
void hosted_is_bool(VM* vm)
{
    set_bool(circa_output(vm), is_bool(circa_input(vm, 0)));
}
void hosted_is_string(VM* vm)
{
    set_bool(circa_output(vm), is_string(circa_input(vm, 0)));
}
void hosted_is_func(VM* vm)
{
    set_bool(circa_output(vm), is_func(circa_input(vm, 0)));
}
void hosted_is_type(VM* vm)
{
    set_bool(circa_output(vm), is_type(circa_input(vm, 0)));
}

void from_string(VM* vm)
{
    parse_string_repr(circa_input(vm, 0), circa_output(vm));
}

void to_string_repr(VM* vm)
{
    write_string_repr(circa_input(vm, 0), circa_output(vm));
}

void test_oracle(VM* vm)
{
    if (list_length(g_oracleValues) == 0)
        set_null(vm->output());
    else {
        copy(list_get(g_oracleValues, 0), vm->output());
        list_remove_index(g_oracleValues, 0);
    }
}

void test_spy(VM* vm)
{
    if (g_spyValues == NULL) {
        g_spyValues = circa_alloc_value();
        set_list(g_spyValues);
    }
    copy(circa_input(vm, 0), list_append(g_spyValues));
}

void test_spy_clear()
{
    if (g_spyValues == NULL)
        g_spyValues = circa_alloc_value();
    set_list(g_spyValues, 0);
}
Value* test_spy_get_results()
{
    return g_spyValues;
}
void test_oracle_clear()
{
    if (g_oracleValues == NULL)
        g_oracleValues = circa_alloc_value();
    set_list(g_oracleValues, 0);
}
void test_oracle_send(Value* value)
{
    copy(value, list_append(g_oracleValues));
}
Value* test_oracle_append()
{
    return list_append(g_oracleValues);
}
void test_oracle_send(int i)
{
    set_int(list_append(g_oracleValues), i);
}

#if 0
void global_script_version(Stack* stack)
{
    World* world = stack->world;
    set_int(circa_output(stack, 0), world->globalScriptVersion);
}
#endif

void method_lookup(VM* vm)
{
    Value* location = circa_input(vm, 0);
    Value* obj = circa_input(vm, 1);
    Value* methodName = circa_input(vm, 2);

    Value nameLocation;
    nameLocation.set_list(2);
    nameLocation.index(0)->set(methodName);
    nameLocation.index(1)->set(location);

    Block* block = find_method_on_type(get_value_type(obj), &nameLocation);

    if (block == NULL)
        set_list(vm->output(), 0);
    else {
        set_closure(vm->output()->set_list(1)->index(0), block, NULL);
    }
}

void get_field(VM* vm)
{
    Value* head = vm->input(0);

    Value error;
    Value* value = get_field(head, vm->input(1), &error);

    if (!is_null(&error)) {
        vm->throw_error(&error);
        return;
    }

    ca_assert(value != NULL);

    copy(value, vm->output());
}

#if 0
void has_method(Stack* stack)
{
    Value* object = circa_input(stack, 0);
    Value* field = circa_input(stack, 1);

    Term* method = find_method(frame_block(top_frame_parent(stack)),
        (Type*) circa_type_of(object), field);...

    if (method != NULL)

    if (is_hashtable(object) && hashtable_get(object, field) != NULL)
        return true;
}
#endif

void get_index(VM* vm)
{
    Value* list = vm->input(0);
    int index = vm->input(1)->as_i();

    if (index < 0) {
        char indexStr[40];
        sprintf(indexStr, "Negative index: %d", index);
        return circa_throw(vm, indexStr);
    } else if (index >= list_length(list)) {
        char indexStr[40];
        sprintf(indexStr, "Index out of range: %d", index);
        return circa_throw(vm, indexStr);
    }

    Value* result = get_index(list, index);

    copy(result, vm->output());
}

void make_list(VM* vm)
{
    // Variadic arg handling will already have turned this into a list.
    move(vm->input(0), vm->output());
}

void make_table(VM* vm)
{
    Value* out = vm->output();
    Value* args = vm->input(0);
    int len = list_length(args);
    if ((len % 2) != 0)
        return vm->throw_str("Number of arguments must be even");

    set_hashtable(out);
    for (int i=0; i < len; i += 2) {
        Value* key = list_get(args, i);
        Value* val = list_get(args, i+1);
        move(val, hashtable_insert(out, key, true));
    }
}

void blank_list(VM* vm)
{
    circa_set_list(vm->output(), vm->input(0)->as_i());
}

void and_func(VM* vm)
{
    set_bool(circa_output(vm),
        circa_input(vm, 0)->asBool() && circa_input(vm, 1)->asBool());
}

void or_func(VM* vm)
{
    set_bool(circa_output(vm),
        circa_input(vm, 0)->asBool() || circa_input(vm, 1)->asBool());
}

void not_func(VM* vm)
{
    set_bool(circa_output(vm), !circa_input(vm, 0)->asBool());
}

void make_func(VM* vm)
{
    make(as_type(circa_input(vm, 0)), circa_output(vm));
}

void max_f(VM* vm)
{
    set_float(circa_output(vm),
            std::max(vm->input(0)->to_f(), vm->input(1)->to_f()));
}

void max_i(VM* vm)
{
    set_int(circa_output(vm),
            std::max(vm->input(0)->as_i(), vm->input(1)->as_i()));
}

void min_f(VM* vm)
{
    set_float(circa_output(vm),
            std::min(vm->input(0)->to_f(), vm->input(1)->to_f()));
}

void min_i(VM* vm)
{
    set_int(circa_output(vm),
            std::min(vm->input(0)->as_i(), vm->input(1)->as_i()));
}

void remainder_i(VM* vm)
{
    set_int(circa_output(vm), vm->input(0)->as_i() % vm->input(1)->as_i());
}

void remainder_f(VM* vm)
{
    set_float(circa_output(vm), fmodf(vm->input(0)->to_f(), vm->input(1)->to_f()));
}

// We compute mod() using floored division. This is different than C and many
// C-like languages which use truncated division. See this page for an explanation
// of the difference:
// http://en.wikipedia.org/wiki/Modulo_operation
//
// For a function that works the same as C's modulo, use remainder() . The % operator
// also uses remainder(), so that it works the same as C's % operator.

void mod_i(VM* vm)
{
    int a = vm->input(0)->as_i();
    int n = vm->input(1)->as_i();

    int out = a % n;
    if (out < 0)
        out += n;

    set_int(circa_output(vm), out);
}

void mod_f(VM* vm)
{
    float a = vm->input(0)->to_f();
    float n = vm->input(1)->to_f();

    float out = fmodf(a, n);

    if (out < 0)
        out += n;

    set_float(circa_output(vm), out);
}

void round(VM* vm)
{
    float input = vm->input(0)->to_f();
    if (input > 0.0)
        set_int(circa_output(vm), int(input + 0.5));
    else
        set_int(circa_output(vm), int(input - 0.5));
}

void floor(VM* vm)
{
    set_int(circa_output(vm), (int) std::floor(vm->input(0)->to_f()));
}

void ceil(VM* vm)
{
    set_int(circa_output(vm), (int) std::ceil(vm->input(0)->to_f()));
}

void average(VM* vm)
{
    Value* args = circa_input(vm, 0);
    int count = args->length();
    Value* out = circa_output(vm);

    if (count == 0) {
        set_float(out, 0);
        return;
    }

    float sum = 0;
    for (int i=0; i < count; i++)
        sum += to_float(circa_index(args, i));

    set_float(out, sum / count);
}

void pow(VM* vm)
{
    set_float(circa_output(vm),
            std::pow((float) to_float(circa_input(vm, 0)), to_float(circa_input(vm, 1))));
}

void sqr(VM* vm)
{
    float in = vm->input(0)->to_f();
    set_float(circa_output(vm), in * in);
}
void cube(VM* vm)
{
    float in = vm->input(0)->to_f();
    set_float(circa_output(vm), in * in * in);
}

void sqrt(VM* vm)
{
    set_float(circa_output(vm), std::sqrt(vm->input(0)->to_f()));
}

void log(VM* vm)
{
    set_float(circa_output(vm), std::log(vm->input(0)->to_f()));
}

void mult_f(VM* vm)
{
    float product = vm->input(0)->to_f() * vm->input(1)->to_f();
    set_float(circa_output(vm), product);
}

void mult_i(VM* vm)
{
    int product = vm->input(0)->as_i() * vm->input(1)->as_i();
    set_int(circa_output(vm), product);
}

void neg_f(VM* vm)
{
    set_float(circa_output(vm), -vm->input(0)->to_f());
}

void neg_i(VM* vm)
{
    set_int(circa_output(vm), -vm->input(0)->as_i());
}

void sub_i(VM* vm)
{
    set_int(circa_output(vm), vm->input(0)->as_i() - vm->input(1)->as_i());
}

void sub_f(VM* vm)
{
    set_float(circa_output(vm), vm->input(0)->to_f() - vm->input(1)->to_f());
}

float radians_to_degrees(float radians) { return radians * 180.0f / M_PI; }
float degrees_to_radians(float unit) { return unit * M_PI / 180.0f; }

void sin_func(VM* vm)
{
    float input = vm->input(0)->to_f();
    set_float(circa_output(vm), sin(degrees_to_radians(input)));
}
void cos_func(VM* vm)
{
    float input = vm->input(0)->to_f();
    set_float(circa_output(vm), cos(degrees_to_radians(input)));
}
void tan_func(VM* vm)
{
    float input = vm->input(0)->to_f();
    set_float(circa_output(vm), tan(degrees_to_radians(input)));
}
void arcsin_func(VM* vm)
{
    float result = asin(vm->input(0)->to_f());
    set_float(circa_output(vm), radians_to_degrees(result));
}
void arccos_func(VM* vm)
{
    float result = acos(vm->input(0)->to_f());
    set_float(circa_output(vm), radians_to_degrees(result));
}
void arctan_func(VM* vm)
{
    float result = atan(vm->input(0)->to_f());
    set_float(circa_output(vm), radians_to_degrees(result));
}

void path_dirname(VM* vm)
{
    Value* path = vm->input(0);
    get_directory_for_filename(path, vm->output());
}

void path_join(VM* vm)
{
    Value* left = vm->output();
    Value* args = vm->input(0);
    if (args->length() == 0) {
        set_string(left, "");
        return;
    }

    copy(args->index(0), left);

    for (int i=1; i < args->length(); i++) {
        join_path(left, args->index(i));
    }
}

void set_field(VM* vm)
{
    stat_increment(SetField);

    Value* out = vm->output();
    copy(vm->input(0), out);
    touch(out);

    Value* name = vm->input(1);

    Value* slot = get_field(out, name, NULL);
    if (slot == NULL) {
        Value msg;
        set_string(&msg, "Field not found: ");
        string_append(&msg, name);
        return vm->throw_error(&msg);
    }

    copy(vm->input(2), slot);
}

void set_index(VM* vm)
{
    stat_increment(SetIndex);

    Value* output = vm->output();
    copy(vm->input(0), output);
    touch(output);
    int index = vm->input(1)->as_i();
    move(vm->input(2), list_get(output, index));
}

void rand(VM* vm)
{
    set_float(vm->output(), rand_next_double(&vm->randState));
}

void repeat(VM* vm)
{
    Value* source = vm->input(0);
    int repeatCount = vm->input(1)->as_i();

    Value* out = vm->output();
    circa_set_list(out, repeatCount);

    for (int i=0; i < repeatCount; i++)
        copy(source, circa_index(out, i));
}

void less_than_i(VM* vm)
{
    vm->output()->set_bool( vm->input(0)->as_i() < vm->input(1)->as_i() );
}

void less_than_f(VM* vm)
{
    vm->output()->set_bool( vm->input(0)->as_f() < vm->input(1)->as_f() );
}

void less_than_eq_i(VM* vm)
{
    vm->output()->set_bool( vm->input(0)->as_i() <= vm->input(1)->as_i() );
}

void less_than_eq_f(VM* vm)
{
    vm->output()->set_bool( vm->input(0)->as_f() <= vm->input(1)->as_f() );
}

void greater_than_i(VM* vm)
{
    vm->output()->set_bool( vm->input(0)->as_i() > vm->input(1)->as_i() );
}

void greater_than_f(VM* vm)
{
    vm->output()->set_bool( vm->input(0)->as_f() > vm->input(1)->as_f() );
}

void greater_than_eq_i(VM* vm)
{
    vm->output()->set_bool( vm->input(0)->as_i() >= vm->input(1)->as_i() );
}

void greater_than_eq_f(VM* vm)
{
    vm->output()->set_bool( vm->input(0)->as_f() >= vm->input(1)->as_f() );
}

void List__append(VM* vm)
{
    Value* out = circa_output(vm);
    move(circa_input(vm, 0), out);
    move(circa_input(vm, 1), list_append(out));
}

void List__concat(VM* vm)
{
    Value* out = circa_output(vm);
    move(circa_input(vm, 0), out);

    Value* additions = circa_input(vm, 1);
    touch(additions);

    int oldLength = list_length(out);
    int additionsLength = list_length(additions);

    list_resize(out, oldLength + additionsLength);
    for (int i = 0; i < additionsLength; i++)
        move(list_get(additions, i), list_get(out, oldLength + i));
}

void List__resize(VM* vm)
{
    Value* out = circa_output(vm);
    copy(circa_input(vm, 0), out);
    int count = circa_input(vm, 1)->as_i();
    circa_resize(out, count);
}

void List__count(VM* vm)
{
    set_int(circa_output(vm), list_length(circa_input(vm, 0)));
}
void List__length(VM* vm)
{
    set_int(circa_output(vm), list_length(circa_input(vm, 0)));
}

void List__insert(VM* vm)
{
    Value* out = circa_output(vm);
    copy(circa_input(vm, 0), out);

    copy(circa_input(vm, 2), list_insert(out, circa_input(vm, 1)->as_i()));
}

void List__slice(VM* vm)
{
    Value* input = circa_input(vm, 0);
    int start = circa_input(vm, 1)->as_i();
    int end = circa_input(vm, 2)->as_i();
    Value* output = circa_output(vm);

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

void List__join(VM* vm)
{
    Value* input = circa_input(vm, 0);
    Value* joiner = circa_input(vm, 1);

    Value* out = circa_output(vm);
    set_string(out, "");

    for (int i=0; i < list_length(input); i++) {
        if (i != 0)
            string_append(out, joiner);

        string_append(out, list_get(input, i));
    }
}

void List__get(VM* vm)
{
    Value* self = circa_input(vm, 0);
    int index = circa_input(vm, 1)->as_i();
    if (index < 0 || index >= list_length(self))
        return circa_throw(vm, "Index out of bounds");

    copy(list_get(self, index), circa_output(vm));
}

void List__set(VM* vm)
{
    Value* self = circa_output(vm);
    move(circa_input(vm, 0), self);
    touch(self);

    int index = circa_input(vm, 1)->as_i();
    Value* value = circa_input(vm, 2);

    move(value, list_get(self, index));
}

void List__remove(VM* vm)
{
    Value* self = circa_output(vm);
    move(circa_input(vm, 0), self);
    int index = circa_input(vm, 1)->as_i();

    if (index < 0 || index >= list_length(self))
        return circa_throw(vm, "Index out of bounds");

    list_remove_index(self, index);
}

void Table__contains(VM* vm)
{
    Value* key = circa_input(vm, 1);
    if (!value_hashable(key))
        return circa_throw(vm, "Key is not hashable");

    Value* value = hashtable_get(circa_input(vm, 0), key);
    set_bool(circa_output(vm), value != NULL);
}

void Table__keys(VM* vm)
{
    Value* table = circa_input(vm, 0);
    hashtable_get_keys(table, circa_output(vm));
}

void Table__remove(VM* vm)
{
    Value* key = circa_input(vm, 1);
    if (!value_hashable(key))
        return circa_throw(vm, "Key is not hashable");

    Value* self = circa_output(vm);
    move(circa_input(vm, 0), self);
    hashtable_remove(self, key);
}

void Table__get(VM* vm)
{
    Value* table = circa_input(vm, 0);
    Value* key = circa_input(vm, 1);
    if (!value_hashable(key))
        return circa_throw(vm, "Key is not hashable");

    Value* value = hashtable_get(table, key);
    if (value == NULL) {
        Value msg;
        set_string(&msg, "Key not found: ");
        string_append_quoted(&msg, key);
        return vm->throw_error(&msg);
    }
    copy(value, circa_output(vm));
}

void Table__set(VM* vm)
{
    Value* key = circa_input(vm, 1);
    if (!value_hashable(key))
        return circa_throw(vm, "Key is not hashable");

    Value* self = circa_output(vm);
    move(circa_input(vm, 0), self);

    Value* value = circa_input(vm, 2);
    move(value, hashtable_insert(self, key, false));
}

void Table__empty(VM* vm)
{
    set_bool(circa_output(vm), hashtable_is_empty(circa_input(vm, 0)));
}

void Module__block(VM* vm)
{
    Value* moduleRef = circa_input(vm, 0);
    Block* moduleBlock = module_ref_resolve(vm->world, moduleRef);
    set_block(circa_output(vm), moduleBlock);
}

void Module__get(VM* vm)
{
    Value* moduleRef = circa_input(vm, 0);
    Block* moduleBlock = module_ref_resolve(vm->world, moduleRef);
    Term* term = find_local_name(moduleBlock, circa_input(vm, 1));
    if (term == NULL)
        set_list(vm->output(), 0);
    else
        copy(term_value(term), vm->output()->set_list(1)->index(0));
}

void String__char_at(VM* vm)
{
    const char* str = circa_input(vm, 0)->as_str();
    int index = circa_input(vm, 1)->as_i();

    if (index < 0) {
        circa_throw(vm, "negative index");
        return;
    }

    if ((unsigned) index >= strlen(str)) {
        set_string(circa_output(vm), "");
        return;
    }

    char output[1];
    output[0] = str[index];
    set_string(circa_output(vm), output, 1);
}

void String__length(VM* vm)
{
    const char* str = circa_input(vm, 0)->as_str();
    set_int(circa_output(vm), (int) strlen(str));
}

void String__char_code(VM* vm)
{
    const char* str = circa_input(vm, 0)->as_str();
    if (strlen(str) != 1)
        return circa_throw(vm, "Expected a string of length 1");
    
    set_int(circa_output(vm), (int) str[0]);
}

void String__from_char_code(VM* vm)
{
    char str[2];
    str[0] = circa_input(vm, 1)->as_i();
    str[1] = 0;
    set_string(circa_output(vm), str);
}

void String__substr(VM* vm)
{
    Value* self = circa_input(vm, 0);
    int start = circa_input(vm, 1)->as_i();
    int length = circa_input(vm, 2)->as_i();

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

    set_string(circa_output(vm), as_cstring(self) + start, length);
}

char character_to_lower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c + 'a' - 'A';
    return c;
}

void String__to_camel_case(VM* vm)
{
    const char* in = circa_input(vm, 0)->as_str();
    set_string(circa_output(vm), in);

    char* out = (char*) as_cstring(circa_output(vm));
    if (out[0] == 0)
        return;

    out[0] = character_to_lower(out[0]);
}

void String__to_lower(VM* vm)
{
    const char* in = circa_input(vm, 0)->as_str();
    int len = (int) strlen(in);

    set_string(circa_output(vm), in);
    char* out = (char*) as_cstring(circa_output(vm));

    for (int i=0; i < len; i++) {
        char c = in[i];

        if (c >= 'A' && c <= 'Z')
            c = c + 'a' - 'A';
        out[i] = c;
    }
}

void String__to_number(VM* vm)
{
    float n = atof(circa_input(vm, 0)->as_str());
    set_float(circa_output(vm), n);
}

void String__to_int(VM* vm)
{
    int n = atoi(circa_input(vm, 0)->as_str());
    set_int(circa_output(vm), n);
}

void String__to_upper(VM* vm)
{
    const char* in = circa_input(vm, 0)->as_str();
    int len = (int) strlen(in);

    set_string(circa_output(vm), in);
    char* out = (char*) as_cstring(circa_output(vm));

    for (int i=0; i < len; i++) {
        char c = in[i];

        if (c >= 'a' && c <= 'z')
            c = c + 'A' - 'a';
        out[i] = c;
    }
}

void String__slice(VM* vm)
{
    int start = circa_input(vm, 1)->as_i();
    int end = circa_input(vm, 2)->as_i();
    std::string const& s = as_string(circa_input(vm, 0));

    // Negative indexes are relatve to end of string
    if (start < 0) start = (int) s.length() + start;
    if (end < 0) end = (int) s.length() + end;

    if (start < 0) return set_string(circa_output(vm), "");
    if (end < 0) return set_string(circa_output(vm), "");

    if ((unsigned) start > s.length())
        start = (int) s.length();

    if ((unsigned) end > s.length())
        end = (int) s.length();

    if (end < start)
        return set_string(circa_output(vm), "");

    set_string(circa_output(vm), s.substr(start, end - start));
}

void String__ends_with(VM* vm)
{
    set_bool(circa_output(vm), string_ends_with(circa_input(vm, 0), as_cstring(circa_input(vm, 1))));
}
void String__starts_with(VM* vm)
{
    set_bool(circa_output(vm), string_starts_with(circa_input(vm, 0), as_cstring(circa_input(vm, 1))));
}

void String__split(VM* vm)
{
    string_split(circa_input(vm, 0), string_get(circa_input(vm, 1), 0), circa_output(vm));
}

#if 0
void file__exists(Stack* stack)
{
    set_bool(circa_output(stack, 0),
        circa_file_exists(stack->world, circa_string_input(stack, 0)));
}
void file__version(Stack* stack)
{
    set_int(circa_output(stack, 0),
        circa_file_get_version(stack->world, circa_string_input(stack, 0)));
}

void file__read_text(Stack* stack)
{
    circa_read_file(stack->world, circa_string_input(stack, 0), circa_output(stack, 0));
}
#endif

void typeof_func(VM* vm)
{
    set_type(vm->output(), get_value_type(vm->input(0)));
}

void length(VM* vm)
{
    set_int(circa_output(vm), num_elements(circa_input(vm, 0)));
}

void noise(VM* vm)
{
    const int octaves = 4;
    float out = perlin_fbm(octaves, vm->input(0)->to_f());
    set_float(vm->output(), out);
}

void not_equals(VM* vm)
{
    set_bool(circa_output(vm),
            !equals(circa_input(vm, 0), circa_input(vm, 1)));
}

void error(VM* vm)
{
    Value* args = circa_input(vm, 0);

    Value out;

    for (int i = 0; i < args->length(); i++) {
        Value* val = circa_index(args, i);
        string_append(&out, val);
    }

    vm->throw_error(&out);
}

void error_up(VM* vm)
{
    Value* height = circa_input(vm, 0);
    Value* args = circa_input(vm, 1);

    Value out;
    for (int i = 0; i < args->length(); i++) {
        Value* val = circa_index(args, i);
        string_append(&out, val);
    }
    vm->throw_error_height(height->asInt(), &out);
}

void get_with_symbol(VM* vm)
{
    Value* left = vm->input(0);
    Value str;
    symbol_to_string(vm->input(1), &str);

    if (is_module_ref(left)) {
        Block* block = module_ref_resolve(vm->world, left);
        Term* term = find_local_name(block, &str);

        if (term != NULL) {
            set_closure(vm->output(), term->nestedContents, NULL);
            return;
        }
    }

    vm->throw_str("Symbol not found");
}

void print(VM* vm)
{
    Value out;
    set_string(&out, "");

    Value* args = vm->input(0);

    for (int i = 0; i < args->length(); i++) {
        Value* val = args->index(i);
        string_append(&out, val);
    }

    write_log(as_cstring(&out));
}

bool set_field(VM* vm, Value* object, Value* name, Value* val)
{
    // preconditions:
    //   'object' is writeable
    //   'val' is safe to consume
    
    if (is_hashtable(object)) {
        move(val, hashtable_insert(object, name));
        return true;
    }

    if (is_list_based_type(object->value_type)) {
        Type* type = object->value_type;
        int fieldIndex = list_find_field_index_by_name(type, name);
        if (fieldIndex == -1) {
            Value msg;
            set_string(&msg, "Field not found (");
            string_append(&msg, name);
            string_append(&msg, ")");
            vm->throw_error(&msg);
            return false;
        }

        Value* slot = list_get(object, fieldIndex);

        move(val, slot);

        Type* fieldType = compound_type_get_field_type(type, fieldIndex);
        if (!cast(slot, fieldType)) {
            Value msg;
            set_string(&msg, "Couldn't cast value ");
            string_append_quoted(&msg, slot);
            string_append(&msg, " to type ");
            string_append(&msg, &fieldType->name);
            string_append(&msg, " (field ");
            string_append_quoted(&msg, name);
            string_append(&msg, " of type ");
            string_append(&msg, &type->name);
            string_append(&msg, ")");
            vm->throw_error(&msg);
            return false;
        }

        return true;
    }

    Value msg;
    set_string(&msg, "Can't assign a field to value ");
    string_append_quoted(&msg, object);
    vm->throw_error(&msg);
    return false;
}

void set_func(VM* vm)
{
    Value* obj = vm->input(0);
    Value* args = vm->input(1);
    int numArgs = args->length();
    if ((numArgs % 2) != 0)
        return vm->throw_str("Number of arguments must be even");

    touch(obj);
    for (int i=0; i < numArgs; i += 2) {
        if (!set_field(vm, obj, args->index(i), args->index(i+1)))
            break;
    }

    move(obj, vm->output());
}

void get_func(VM* vm)
{
    Value* obj = vm->input(0);
    Value* name = vm->input(1);
    Value* found = NULL;
    Type* type = obj->value_type;

    if (is_hashtable(obj)) {
        found = hashtable_get(obj, name);
        if (found == NULL) {
            Value msg;
            set_string(&msg, "Field not found (");
            string_append(&msg, name);
            string_append(&msg, ")");
            vm->throw_error(&msg);
            return;
        }

        copy(found, vm->output());
        return;
    }
    
    if (is_list_based_type(type)) {
        int fieldIndex = list_find_field_index_by_name(type, name);
        if (fieldIndex == -1) {
            Value msg;
            set_string(&msg, "Field not found (");
            string_append(&msg, name);
            string_append(&msg, ")");
            vm->throw_error(&msg);
            return;
        }
        found = list_get(obj, fieldIndex);
        copy(found, vm->output());
        return;
    }

    Value msg;
    set_string(&msg, "Can't get field from value ");
    string_append_quoted(&msg, obj);
    vm->throw_error(&msg);
}

void compute_patch_hosted(VM* vm)
{
    Value error;

    compute_value_patch(circa_input(vm, 0), circa_input(vm, 1),
        circa_output(vm), &error);

    if (!is_null(&error))
        circa_output_error_val(vm, &error);
}

void apply_patch_hosted(VM* vm)
{
    Value* result = circa_output(vm);
    copy(circa_input(vm, 0), result);
    apply_patch(result, circa_input(vm, 1));
}

void unique_id(VM* vm)
{
    int id = vm->nextUniqueId++;
    set_int(vm->output(), id);
}

void cache_get(VM* vm)
{
    Value* key = vm->input(0);
    Value* found = hashtable_get(&vm->cache, key);
    if (found == NULL)
        set_list(vm->output(), 0);
    else
        set_list_1(vm->output(), found);
}

void cache_set(VM* vm)
{
    Value* key = vm->input(0);
    Value* val = vm->input(1);
    move(val, hashtable_insert(&vm->cache, key));
}

void make_module(VM* vm)
{
    Value* name = vm->input(0);
    Value* relativeTo = vm->input(1);

    Value resolved;
    if (is_block(relativeTo))
        find_enclosing_dirname(as_block(relativeTo), &resolved);
    else
        move(relativeTo, &resolved);
    
    set_module_ref(vm->output(), name, &resolved);
    load_module(vm->world, &resolved, name);
}

#if 0
void destructure_list(Stack* stack)
{
    Value* list = circa_input(stack, 0);
    int declaredCount = circa_int_input(stack, 1);
    int listLength = list->length();
    for (int i=0; i < declaredCount; i++) {
        if (i >= listLength)
            set_null(circa_output(stack, i));
        else
            copy(list->index(i), circa_output(stack, i));
    }
}
#endif

void misc_builtins_setup_functions(NativePatch* patch)
{
    circa_patch_function(patch, "add_i", add_i);
    circa_patch_function(patch, "add_f", add_f);
    circa_patch_function(patch, "abs", abs);
    circa_patch_function(patch, "assert", assert_func);
    circa_patch_function(patch, "cast", cast_evaluate);
    circa_patch_function(patch, "str", str);
    circa_patch_function(patch, "cond", cond);
    circa_patch_function(patch, "copy", copy_eval);
    circa_patch_function(patch, "div_f", div_f);
    circa_patch_function(patch, "div_i", div_i);
    circa_patch_function(patch, "empty_list", empty_list);
    circa_patch_function(patch, "equals", equals_func);
    circa_patch_function(patch, "error", error);
    circa_patch_function(patch, "error_up", error_up);
    circa_patch_function(patch, "method_lookup", method_lookup);
    circa_patch_function(patch, "get_field", get_field);
    circa_patch_function(patch, "get_index", get_index);
    circa_patch_function(patch, "get_with_symbol", get_with_symbol);
    circa_patch_function(patch, "is_compound", hosted_is_compound);
    circa_patch_function(patch, "is_list", hosted_is_list);
    circa_patch_function(patch, "is_int", hosted_is_int);
    circa_patch_function(patch, "is_table", hosted_is_table);
    circa_patch_function(patch, "is_number", hosted_is_number);
    circa_patch_function(patch, "is_bool", hosted_is_bool);
    circa_patch_function(patch, "is_string", hosted_is_string);
    circa_patch_function(patch, "is_func", hosted_is_func);
    circa_patch_function(patch, "is_type", hosted_is_type);
    circa_patch_function(patch, "length", length);
    circa_patch_function(patch, "less_than_i", less_than_i);
    circa_patch_function(patch, "less_than_f", less_than_f);
    circa_patch_function(patch, "less_than_eq_i", less_than_eq_i);
    circa_patch_function(patch, "less_than_eq_f", less_than_eq_f);
    circa_patch_function(patch, "greater_than_i", greater_than_i);
    circa_patch_function(patch, "greater_than_f", greater_than_f);
    circa_patch_function(patch, "greater_than_eq_i", greater_than_eq_i);
    circa_patch_function(patch, "greater_than_eq_f", greater_than_eq_f);
    circa_patch_function(patch, "make_list", make_list);
    circa_patch_function(patch, "blank_list", blank_list);
    circa_patch_function(patch, "make_table", make_table);
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

    circa_patch_function(patch, "path_dirname", path_dirname);
    circa_patch_function(patch, "path_join", path_join);
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

    circa_patch_function(patch, "Table.contains", Table__contains);
    circa_patch_function(patch, "Table.keys", Table__keys);
    circa_patch_function(patch, "Table.remove", Table__remove);
    circa_patch_function(patch, "Table.get", Table__get);
    circa_patch_function(patch, "Table.set", Table__set);
    circa_patch_function(patch, "Table.empty", Table__empty);

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

#if 0
    circa_patch_function(patch, "file_version", file__version);
    circa_patch_function(patch, "file_exists", file__exists);
    circa_patch_function(patch, "file_read_text", file__read_text);
#endif
    circa_patch_function(patch, "make_module", make_module);

    circa_patch_function(patch, "noise", noise);
    circa_patch_function(patch, "not_equals", not_equals);
    circa_patch_function(patch, "print", print);
    circa_patch_function(patch, "rand", rand);
    circa_patch_function(patch, "repeat", repeat);
    circa_patch_function(patch, "trace", print);
    circa_patch_function(patch, "typeof", typeof_func);
    circa_patch_function(patch, "set", set_func);
    circa_patch_function(patch, "get", get_func);
    circa_patch_function(patch, "compute_patch", compute_patch_hosted);
    circa_patch_function(patch, "apply_patch", apply_patch_hosted);
    circa_patch_function(patch, "unique_id", unique_id);
    circa_patch_function(patch, "cache_get", cache_get);
    circa_patch_function(patch, "cache_set", cache_set);
#if 0
    circa_patch_function(patch, "source_id", source_id);
#endif
    circa_patch_function(patch, "from_string", from_string);
    circa_patch_function(patch, "to_string_repr", to_string_repr);
    circa_patch_function(patch, "test_spy", test_spy);
    circa_patch_function(patch, "test_oracle", test_oracle);

#if 0
    circa_patch_function(patch, "global_script_version", global_script_version);
#endif
}

} // namespace circa
