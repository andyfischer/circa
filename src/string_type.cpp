// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <cstring>

#include "kernel.h"
#include "importing.h"
#include "evaluation.h"
#include "list.h"
#include "string_type.h"
#include "source_repro.h"
#include "names.h"
#include "tagged_value.h"
#include "token.h"
#include "type.h"

namespace circa {

struct StringData {
    int refCount;
    int length;
    char str[0];
};

void incref(StringData* data)
{
    data->refCount++;
}

void decref(StringData* data)
{
    ca_assert(data->refCount > 0);
    data->refCount--;
    if (data->refCount == 0) {
        free(data);
    }
}

// Create a new blank string with the given length. Starts off with 1 ref.
StringData* string_create(int length)
{
    StringData* result = (StringData*) malloc(sizeof(StringData) + length + 1);
    result->refCount = 1;
    result->length = length;
    result->str[0] = 0;
    return result;
}

// Creates a hard duplicate of a string. Starts off with 1 ref.
StringData* string_duplicate(StringData* original)
{
    StringData* dup = string_create(original->length);
    memcpy(dup->str, original->str, original->length + 1);
    return dup;
}

// Make sure that this StringData* is safe to modify. It may leave the pointer
// untouched. It may create a new StringData, modify the pointer, and decref
// the old data.
void string_touch(StringData** data)
{
    ca_assert((*data)->refCount > 0);
    if ((*data)->refCount == 1)
        return;

    StringData* dup = string_duplicate(*data);
    decref(*data);
    *data = dup;
}

void string_resize(StringData** data, int newLength)
{
    if (*data == NULL) {
        *data = string_create(newLength);
        return;
    }

    // Perform the same check as touch()
    if ((*data)->refCount == 1) {
        // Modify in-place
        *data = (StringData*) realloc(*data, sizeof(StringData) + newLength + 1);
        (*data)->length = newLength;
        return;
    }

    StringData* oldData = *data;
    StringData* newData = string_create(newLength);
    memcpy(newData->str, oldData->str, oldData->length + 1);
    decref(oldData);
    *data = newData;
}

int string_length(StringData* data)
{
    return strlen(data->str);
}

// Tagged-value wrappers
void string_initialize(Type* type, caValue* value)
{
    value->value_data.ptr = NULL;
}

void string_release(caValue* value)
{
    if (value->value_data.ptr == NULL)
        return;
    decref((StringData*) value->value_data.ptr);
}

void string_copy(Type* type, caValue* source, caValue* dest)
{
    create(type, dest);
    StringData* data = (StringData*) source->value_data.ptr;
    if (data != NULL)
        incref(data);
    dest->value_data.ptr = data;
}

void string_reset(caValue* val)
{
    StringData* data = (StringData*) val->value_data.ptr;
    if (data != NULL)
        decref(data);
    val->value_data.ptr = NULL;
}

int string_hash(caValue* val)
{
    const char* str = as_cstring(val);

    // Dumb and simple hash function
    int result = 0;
    int byte = 0;
    while (*str != 0) {
        result = result ^ (*str << (8 * byte));
        byte = (byte + 1) % 4;
        str++;
    }
    return result;
}

bool string_equals(caValue* left, caValue* right)
{
    if (!is_string(right))
        return false;

    // Shortcut, check if objects are the same.
    if (left->value_data.ptr == right->value_data.ptr)
        return true;

    if (left->value_data.ptr == NULL)
        return false;

    const char* leftChars = as_cstring(left);
    const char* rightChars = as_cstring(right);
    for (int i=0;; i++) {
        if (leftChars[i] != rightChars[i])
            return false;
        if (leftChars[i] == 0)
            break;
    }

    return true;
}

std::string string_to_string(caValue* value)
{
    std::stringstream result;
    result << "'" << as_cstring(value) << "'";
    return result.str();
}

void string_format_source(caValue* source, Term* term)
{
    if (term->hasProperty("syntax:originalString")) {
        append_phrase(source, term->stringProp("syntax:originalString", ""),
                term, tok_String);
        return;
    }

    std::string quoteType = term->stringProp("syntax:quoteType", "'");
    std::string result;
    if (quoteType == "<")
        result = "<<<" + as_string(term_value(term)) + ">>>";
    else
        result = quoteType + as_cstring(term_value(term)) + quoteType;

    append_phrase(source, result, term, tok_String);
}

void string_setup_type(Type* type)
{
    reset_type(type);
    type->name = name_from_string("String");
    type->storageType = STORAGE_TYPE_STRING;
    type->initialize = string_initialize;
    type->release = string_release;
    type->copy = string_copy;
    type->equals = string_equals;
    type->hashFunc = string_hash;
    type->toString = string_to_string;
    type->formatSource = string_format_source;
}

const char* as_cstring(caValue* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_STRING);
    StringData* data = (StringData*) value->value_data.ptr;
    if (data == NULL)
        return "";
    return data->str;
}

void string_append(caValue* left, const char* right)
{
    StringData** data = (StringData**) &left->value_data.ptr;
    
    int leftLength = string_length(*data);
    int rightLength = strlen(right);
    int newLength = leftLength + rightLength;

    string_resize(data, newLength);

    memcpy((*data)->str + leftLength, right, rightLength + 1);
}

void string_append(caValue* left, caValue* right)
{
    if (is_string(right))
        string_append(left, as_cstring(right));
    else {
        std::string s = to_string(right);
        string_append(left, s.c_str());
    }
}
void string_append_quoted(caValue* left, caValue* right)
{
    std::string s = to_string(right);
    string_append(left, s.c_str());
}
void string_append(caValue* left, int value)
{
    char buf[64];
    sprintf(buf, "%d", value);
    string_append(left, buf);
}
void string_resize(caValue* s, int length)
{
    if (length < 0)
        length = string_length(s) + length;

    StringData** data = (StringData**) &s->value_data.ptr;
    string_resize(data, length);
}
bool string_eq(caValue* s, const char* str)
{
    return strcmp(as_cstring(s), str) == 0;
}

bool string_starts_with(caValue* s, const char* beginning)
{
    const char* left = as_cstring(s);
    for (int i=0;; i++) {
        if (beginning[i] == 0)
            return true;
        if (left[i] != beginning[i])
            return false;
    }
}

bool string_ends_with(caValue* s, const char* ending)
{
    int ending_len = strlen(ending);
    int s_len = string_length(s);
    const char* left = as_cstring(s);

    for (int i=0; i < ending_len; i++)
        if (left[s_len - ending_len + i] != ending[i])
            return false;

    return true;
}

char string_get(caValue* s, int index)
{
    return as_cstring(s)[index];
}

int string_length(caValue* s)
{
    return string_length((StringData*) s->value_data.ptr);
}

void string_slice(caValue* s, int start, int end, caValue* out)
{
    if (s == out)
        internal_error("Usage error in string_slice, 's' cannot be 'out'");

    if (end == -1)
        end = string_length(s);

    int len = end - start;
    string_resize(out, len);

    StringData* data = (StringData*) s->value_data.ptr;

    set_string(out, data->str + start, len);
}

int string_find_char(caValue* s, int start, char c)
{
    const char* cstr = as_cstring(s);

    for (int i=start; cstr[i] != 0; i++)
        if (cstr[i] == c)
            return i;
    return -1;
}

int string_find_char_from_end(caValue* s, char c)
{
    const char* cstr = as_cstring(s);

    for (int i=strlen(cstr) - 1; i >= 0; i--)
        if (cstr[i] == c)
            return i;
    return -1;
}

void string_split(caValue* s, char sep, caValue* listOut)
{
    set_list(listOut, 0);

    int len = string_length(s);
    int wordStart = 0;
    for (int pos=0; pos <= len; pos++) {
        if (pos == len || string_get(s, pos) == sep) {
            string_slice(s, wordStart, pos, list_append(listOut));
            wordStart = pos + 1;
        }
    }
}

std::string as_string(caValue* value)
{
    return std::string(as_cstring(value));
}

void set_string(caValue* value, const char* s)
{
    create(&STRING_T, value);
    int length = strlen(s);
    StringData* data = string_create(length);
    memcpy(data->str, s, length + 1);
    value->value_data.ptr = data;
}

void set_string(caValue* value, const char* s, int length)
{
    create(&STRING_T, value);
    StringData* data = string_create(length);
    memcpy(data->str, s, length);
    data->str[length] = 0;
    value->value_data.ptr = data;
}

char* circa_strdup(const char* s)
{
    size_t length = strlen(s);
    char* out = (char*) malloc(length + 1);
    memcpy(out, s, length + 1);
    return out;
}

} // namespace circa
