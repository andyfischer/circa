// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "closures.h"
#include "debug.h"
#include "hashtable.h"
#include "interpreter.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "native_patch.h"
#include "rand.h"
#include "stack.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"

namespace circa {

void abs(caStack* stack)
{
    set_float(circa_output(stack, 0), std::abs(circa_float_input(stack, 0)));
}

void assert_func(caStack* stack)
{
    if (!circa_bool_input(stack, 0))
        circa_output_error(stack, "Assert failed");
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

void equals_func(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            equals(circa_input(stack, 0), circa_input(stack, 1)));
}

void hosted_is_compound(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_list_storage(circa_input(stack, 0)));
}

void hosted_is_list(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_list2(circa_input(stack, 0)));
}
void hosted_is_int(caStack* stack)
{
    set_bool(circa_output(stack, 0), is_int2(circa_input(stack, 0)));
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

void rand(caStack* stack)
{
    set_float(circa_output(stack, 0), rand_next_double(&stack->randState));
}

void repeat(caStack* stack)
{
    caValue* source = circa_input(stack, 0);
    int repeatCount = circa_int_input(stack, 1);

    caValue* out = circa_output(stack, 0);
    circa_set_list(out, repeatCount);

    for (int i=0; i < repeatCount; i++)
        copy(source, circa_index(out, i));
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

    copy(circa_input(stack, 2), list_insert(out, circa_int_input(stack, 1)));
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

void List__remove(caStack* stack)
{
    caValue* self = circa_output(stack, 0);
    copy(circa_input(stack, 0), self);
    int index = circa_int_input(stack, 1);

    if (index < 0 || index >= list_length(self))
        return circa_output_error(stack, "Invalid index");

    list_remove_index(self, index);
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

void Map__empty(caStack* stack)
{
    set_bool(circa_output(stack, 0), hashtable_is_empty(circa_input(stack, 0)));
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

caValue* find_env_value(caStack* stack, caValue* key)
{
    Frame* frame = top_frame(stack);

    while (frame != NULL) {
        if (!is_null(&frame->dynamicScope)) {
            caValue* value = hashtable_get(&frame->dynamicScope, key);
            if (value != NULL)
                return value;
        }

        frame = prev_frame(frame);
    }

    if (!is_null(&stack->env)) {
        caValue* value = hashtable_get(&stack->env, key);
        if (value != NULL)
            return value;
    }

    return NULL;
}

void get_env(caStack* stack)
{
    caValue* value = find_env_value(stack, circa_input(stack, 0));
    if (value != NULL)
        copy(value, circa_output(stack, 0));
    else
        set_null(circa_output(stack, 0));
}

void get_env_opt(caStack* stack)
{
    caValue* value = find_env_value(stack, circa_input(stack, 0));
    if (value != NULL)
        copy(value, circa_output(stack, 0));
    else
        copy(circa_input(stack, 1), circa_output(stack, 0));
}

void set_env(caStack* stack)
{
    caValue* key = circa_input(stack, 0);
    caValue* value = circa_input(stack, 1);

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

void not_equals(caStack* stack)
{
    set_bool(circa_output(stack, 0),
            !equals(circa_input(stack, 0), circa_input(stack, 1)));
}

void error(caStack* stack)
{
    caValue* args = circa_input(stack, 0);

    std::stringstream out;

    for (int i = 0; i < circa_count(args); i++) {
        caValue* val = circa_index(args, i);
        if (is_string(val))
            out << as_string(val);
        else
            out << to_string(val);
    }

    circa_output_error(stack, out.str().c_str());
}

void get_with_symbol(caStack* stack)
{
    caValue* left = circa_input(stack, 0);
    Value str;
    symbol_as_string(circa_input(stack, 1), &str);

    if (is_module_ref(left)) {
        Block* block = module_ref_get_block(left);
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
    caValue* args = circa_input(stack, 0);

    std::stringstream out;

    for (int i = 0; i < circa_count(args); i++) {
        caValue* val = circa_index(args, i);
        if (is_string(val))
            out << as_string(val);
        else
            out << to_string(val);
    }

    write_log(out.str().c_str());

    set_null(circa_output(stack, 0));
}

void to_string(caStack* stack)
{
    caValue* in = circa_input(stack, 0);
    caValue* out = circa_output(stack, 0);
    if (is_string(in))
        copy(in, out);
    else
        set_string(out, to_string(in));
}

void misc_builtins_setup_functions(NativePatch* patch)
{
    module_patch_function(patch, "abs", abs);
    module_patch_function(patch, "assert", assert_func);
    module_patch_function(patch, "empty_list", empty_list);
    module_patch_function(patch, "equals", equals_func);
    module_patch_function(patch, "error", error);
    module_patch_function(patch, "get_with_symbol", get_with_symbol);
    module_patch_function(patch, "is_compound", hosted_is_compound);
    module_patch_function(patch, "is_list", hosted_is_list);
    module_patch_function(patch, "is_int", hosted_is_int);
    module_patch_function(patch, "is_number", hosted_is_number);
    module_patch_function(patch, "is_bool", hosted_is_bool);
    module_patch_function(patch, "is_string", hosted_is_string);
    module_patch_function(patch, "is_null", hosted_is_null);
    module_patch_function(patch, "is_function", hosted_is_function);
    module_patch_function(patch, "is_type", hosted_is_type);
    module_patch_function(patch, "length", length);
    module_patch_function(patch, "List.append", List__append);
    module_patch_function(patch, "List.concat", List__concat);
    module_patch_function(patch, "List.resize", List__resize);
    module_patch_function(patch, "List.count", List__count);
    module_patch_function(patch, "List.insert", List__insert);
    module_patch_function(patch, "List.length", List__length);
    module_patch_function(patch, "List.join", List__join);
    module_patch_function(patch, "List.slice", List__slice);
    module_patch_function(patch, "List.get", List__get);
    module_patch_function(patch, "List.set", List__set);
    module_patch_function(patch, "List.remove", List__remove);
    module_patch_function(patch, "Map.contains", Map__contains);
    module_patch_function(patch, "Map.remove", Map__remove);
    module_patch_function(patch, "Map.get", Map__get);
    module_patch_function(patch, "Map.set", Map__set);
    module_patch_function(patch, "Map.insertPairs", Map__insertPairs);
    module_patch_function(patch, "Map.empty", Map__empty);
    module_patch_function(patch, "Mutable.get", Mutable__get);
    module_patch_function(patch, "Mutable.set", Mutable__set);
    module_patch_function(patch, "String.char_at", String__char_at);
    module_patch_function(patch, "String.ends_with", String__ends_with);
    module_patch_function(patch, "String.length", String__length);
    module_patch_function(patch, "String.substr", String__substr);
    module_patch_function(patch, "String.slice", String__slice);
    module_patch_function(patch, "String.starts_with", String__starts_with);
    module_patch_function(patch, "String.split", String__split);
    module_patch_function(patch, "String.to_camel_case", String__to_camel_case);
    module_patch_function(patch, "String.to_upper", String__to_upper);
    module_patch_function(patch, "String.to_lower", String__to_lower);

    module_patch_function(patch, "env", get_env);
    module_patch_function(patch, "env_opt", get_env_opt);
    module_patch_function(patch, "set_env", set_env);

    module_patch_function(patch, "file_version", file__version);
    module_patch_function(patch, "file_exists", file__exists);
    module_patch_function(patch, "file_read_text", file__read_text);

    module_patch_function(patch, "not_equals", not_equals);
    module_patch_function(patch, "print", print);
    module_patch_function(patch, "rand", rand);
    module_patch_function(patch, "repeat", repeat);
    module_patch_function(patch, "to_string", to_string);
    module_patch_function(patch, "trace", print);
    module_patch_function(patch, "type", typeof_func);
    module_patch_function(patch, "static_type", static_type_func);
}

} // namespace circa

