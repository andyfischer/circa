// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <cstring>

#include "blob.h"
#include "importing.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "string_type.h"
#include "source_repro.h"
#include "symbols.h"
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
    if (data->refCount == 0)
        free(data);
}

// Create a new blank string with the given length. Starts off with 1 ref.
StringData* string_create(int length)
{
    INCREMENT_STAT(StringCreate);

    StringData* result = (StringData*) malloc(sizeof(StringData) + length + 1);
    result->refCount = 1;
    result->length = length;
    result->str[0] = 0;
    return result;
}

// Creates a hard duplicate of a string. Starts off with 1 ref.
StringData* string_duplicate(StringData* original)
{
    INCREMENT_STAT(StringDuplicate);

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
        INCREMENT_STAT(StringResizeInPlace);

        // Modify in-place
        *data = (StringData*) realloc(*data, sizeof(StringData) + newLength + 1);
        (*data)->str[newLength] = 0;
        (*data)->length = newLength;
        return;
    }

    INCREMENT_STAT(StringResizeCreate);

    StringData* oldData = *data;
    StringData* newData = string_create(newLength);
    memcpy(newData->str, oldData->str, newLength);
    newData->str[newLength] = 0;
    decref(oldData);
    *data = newData;
}

int string_length(StringData* data)
{
    return data->length;
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
    StringData* data = (StringData*) source->value_data.ptr;
    if (data != NULL)
        incref(data);

    make_no_initialize(source->value_type, dest);
    dest->value_data.ptr = data;

    INCREMENT_STAT(StringSoftCopy);
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
    if (left->value_type != right->value_type)
        return false;

    // Shortcut, check if objects are the same.
    if (left->value_data.ptr == right->value_data.ptr)
        return true;

    StringData* leftData = (StringData*) left->value_data.ptr;
    StringData* rightData = (StringData*) right->value_data.ptr;

    if ((leftData == NULL || leftData->str[0] == 0)
        && (rightData == NULL || rightData->str[0] == 0))
        return true;

    if (leftData == NULL || rightData == NULL)
        return false;

    for (int i=0;; i++) {
        if (leftData->str[i] != rightData->str[i])
            return false;
        if (leftData->str[i] == 0)
            break;
    }

    #if CIRCA_ENABLE_SNEAKY_EQUALS
        // Strings are equal. Sneakily have both values reference the same data.
        // Prefer to preserve the one that has more references.
        if (leftData->refCount >= rightData->refCount)
            string_copy(NULL, left, right);
        else
            string_copy(NULL, right, left);
    #endif

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
    if (term->hasProperty(sym_Syntax_OriginalFormat)) {
        append_phrase(source, term->stringProp(sym_Syntax_OriginalFormat, ""),
                term, tok_String);
        return;
    }

    std::string quoteType = term->stringProp(sym_Syntax_QuoteType, "'");
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
    set_string(&type->name, "String");
    type->storageType = sym_StorageTypeString;
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
    ca_assert(value->value_type->storageType == sym_StorageTypeString);
    StringData* data = (StringData*) value->value_data.ptr;
    if (data == NULL)
        return "";
    return data->str;
}

void string_append(caValue* left, const char* right)
{
    string_append_len(left, right, strlen(right));
}

void string_append_len(caValue* left, const char* right, int len)
{
    if (is_null(left))
        set_string(left, "");

    ca_assert(is_string(left));

    StringData** data = (StringData**) &left->value_data.ptr;
    
    int leftLength = string_length(*data);
    int newLength = leftLength + len;

    string_resize(data, newLength);

    memcpy((*data)->str + leftLength, right, len);
    (*data)->str[newLength] = 0;
}

void string_append(caValue* left, caValue* right)
{
    if (is_null(left))
        set_string(left, "");

    ca_assert(is_string(left));

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
    ca_assert(is_string(left));

    char buf[64];
    sprintf(buf, "%d", value);
    string_append(left, buf);
}
void string_append_f(caValue* left, float value)
{
    ca_assert(is_string(left));

    char buf[64];
    sprintf(buf, "%f", value);
    string_append(left, buf);
}
void string_append_char(caValue* left, char c)
{
    char buf[2];
    buf[0] = c;
    buf[1] = 0;
    string_append(left, buf);
}

void string_append_qualified_name(caValue* left, caValue* right)
{
    if (string_equals(left, "")) {
        copy(right, left);
        return;
    }
    if (string_equals(right, ""))
        return;
    string_append(left, ":");
    string_append(left, right);
}

void string_resize(caValue* s, int length)
{
    if (length < 0)
        length = string_length(s) + length;

    StringData** data = (StringData**) &s->value_data.ptr;
    string_resize(data, length);
}
bool string_equals(caValue* s, const char* str)
{
    if (is_string(s))
        return strcmp(as_cstring(s), str) == 0;

    // Preparing for the future, strings and symbols will be one data type.
    if (is_symbol(s)) {
        if (str[0] != ':')
            return false;

        return strcmp(symbol_as_string(s), str + 1) == 0;
    }

    return false;
}
bool string_empty(caValue* s)
{
    return is_null(s) || string_equals(s, "");
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
    int ending_len = (int) strlen(ending);
    int s_len = string_length(s);
    const char* left = as_cstring(s);

    for (int i=0; i < ending_len; i++)
        if (left[s_len - ending_len + i] != ending[i])
            return false;

    return true;
}

void string_remove_suffix(caValue* s, const char* str)
{
    if (!string_ends_with(s, str))
        return;

    string_resize(s, string_length(s) - int(strlen(str)));
}

char string_get(caValue* s, int index)
{
    return as_cstring(s)[index];
}

int string_length(caValue* s)
{
    return string_length((StringData*) s->value_data.ptr);
}

bool string_less_than(caValue* left, caValue* right)
{
    return strcmp(as_cstring(left), as_cstring(right)) < 0;
}

void string_prepend(caValue* result, caValue* prefix)
{
    Value output;
    copy(prefix, &output);
    string_append(&output, result);
    move(&output, result);
}
void string_prepend(caValue* result, const char* prefix)
{
    Value output;
    set_string(&output, prefix);
    string_append(&output, result);
    move(&output, result);
}

void string_slice(caValue* s, int start, int end, caValue* out)
{
    // TODO: This func could be improved to work correctly when 's' is the same object
    // as 'out'.
    
    if (s == out)
        internal_error("Usage error in string_slice, 's' cannot be 'out'");

    if (end == -1)
        end = string_length(s);

    int len = end - start;

    set_string(out, as_cstring(s) + start, len);
}

void string_slice(caValue* str, int start, int end)
{
    touch(str);

    if (end == -1)
        end = string_length(str);

    int newSize = end - start;

    if (start > 0)
        memmove((char*) as_cstring(str), as_cstring(str) + start, newSize);

    string_resize(str, newSize);
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

    for (int i= (int) strlen(cstr) - 1; i >= 0; i--)
        if (cstr[i] == c)
            return i;
    return -1;
}

void string_quote_and_escape(caValue* s)
{
    const char* input = as_cstring(s);

    std::stringstream result;

    result << '"';

    for (int i=0; input[i] != 0; i++) {
        if (input[i] == '\n')
            result << "\\n";
        else if (input[i] == '\'')
            result << "\\'";
        else if (input[i] == '"')
            result << "\\\"";
        else if (input[i] == '\\')
            result << "\\\\";
        else
            result << input[i];
    }

    result << '"';

    set_string(s, result.str());
}

void string_unquote_and_unescape(caValue* s)
{
    if (string_empty(s))
        return;

    const char* input = as_cstring(s);

    char quote = input[0];

    int quoteSize = 1;
    if (quote == '<')
        quoteSize = 3;

    int end = (int) strlen(input) - quoteSize;

    // Unescape any escaped characters
    std::stringstream result;
    for (int i=quoteSize; i < end; i++) {
        char c = input[i];
        char next = 0;
        if (i + 1 < end)
            next = input[i+1];

        if (c == '\\') {
            if (next == 'n') {
                result << '\n';
                i++;
            } else if (next == '\'') {
                result << '\'';
                i++;
            } else if (next == '\"') {
                result << '\"';
                i++;
            } else if (next == '\\') {
                result << '\\';
                i++;
            } else {
                result << c;
            }
        } else {
            result << c;
        }
    }

    set_string(s, result.str());
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
    INCREMENT_STAT(StringToStd);
    return std::string(as_cstring(value));
}

char* string_initialize(caValue* value, int length)
{
    make(TYPES.string, value);
    StringData* data = string_create(length);
    value->value_data.ptr = data;
    return data->str;
}

void set_string(caValue* value, const char* s)
{
    make(TYPES.string, value);
    int length = (int) strlen(s);
    StringData* data = string_create(length);
    memcpy(data->str, s, length + 1);
    value->value_data.ptr = data;
}

void set_string(caValue* value, const char* s, int length)
{
    make(TYPES.string, value);
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

CIRCA_EXPORT int circa_string_length(caValue* string)
{
    return string_length(string);
}

CIRCA_EXPORT int circa_blob_length(caValue* blob)
{
    return blob_size(blob);
}

} // namespace circa
