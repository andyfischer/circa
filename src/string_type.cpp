// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <cstring>

#include "kernel.h"
#include "list.h"
#include "string_type.h"
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
    stat_increment(StringCreate);

    StringData* result = (StringData*) malloc(sizeof(StringData) + length + 1);
    result->refCount = 1;
    result->length = length;
    result->str[0] = 0;
    return result;
}

// Creates a hard duplicate of a string. Starts off with 1 ref.
StringData* string_duplicate(StringData* original)
{
    stat_increment(StringDuplicate);

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
        stat_increment(StringResizeInPlace);

        // Modify in-place
        *data = (StringData*) realloc(*data, sizeof(StringData) + newLength + 1);
        (*data)->str[newLength] = 0;
        (*data)->length = newLength;
        return;
    }

    stat_increment(StringResizeCreate);

    StringData* oldData = *data;
    StringData* newData = string_create(newLength);
    memcpy(newData->str, oldData->str, std::min(newLength, oldData->length));
    newData->str[newLength] = 0;
    decref(oldData);
    *data = newData;
}

int string_length(StringData* data)
{
    if (data == NULL)
        return 0;
    return data->length;
}

// Tagged-value wrappers
void string_initialize(Type* type, Value* value)
{
    value->value_data.ptr = NULL;
}

void string_release(Value* value)
{
    if (value->value_data.ptr == NULL)
        return;
    decref((StringData*) value->value_data.ptr);
}

void string_copy(Value* source, Value* dest)
{
    StringData* data = (StringData*) source->value_data.ptr;
    if (data != NULL)
        incref(data);

    make_no_initialize(source->value_type, dest);
    dest->value_data.ptr = data;

    stat_increment(StringSoftCopy);
}

void string_reset(Value* val)
{
    StringData* data = (StringData*) val->value_data.ptr;
    if (data != NULL)
        decref(data);
    val->value_data.ptr = NULL;
}

int string_hash(Value* val)
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

bool string_equals(Value* left, Value* right)
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
            string_copy(left, right);
        else
            string_copy(right, left);
    #endif

    return true;
}

void string_to_string(Value* value, Value* asStr)
{
    string_append(asStr, "'");
    string_append(asStr, value);
    string_append(asStr, "'");
}

void string_setup_type(Type* type)
{
    reset_type(type);
    set_string(&type->name, "String");
    type->storageType = s_StorageTypeString;
    type->initialize = string_initialize;
    type->release = string_release;
    type->copy = string_copy;
    type->equals = string_equals;
    type->hashFunc = string_hash;
    type->toString = string_to_string;
}

const char* as_cstring(Value* value)
{
    ca_assert(value->value_type->storageType == s_StorageTypeString);
    StringData* data = (StringData*) value->value_data.ptr;
    if (data == NULL)
        return "";
    return data->str;
}

void string_append(Value* left, const char* right)
{
    string_append_len(left, right, (int) strlen(right));
}

void string_append(Value* left, const std::string& right)
{
    string_append_len(left, right.c_str(), (int) right.size());
}

void string_append_len(Value* left, const char* right, int len)
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

void string_append(Value* left, Value* right)
{
    if (is_null(left))
        set_string(left, "");

    ca_assert(is_string(left));

    if (right == NULL)
        return;

    if (is_string(right)) {
        string_append_len(left, as_cstring(right), string_length(right));
    } else {
        to_string(right, left);
    }
}

void string_append_quoted(Value* out, Value* s)
{
    if (is_string(s))
        string_to_string(s, out);
    else
        string_append(out, s);
}

void string_append(Value* left, int value)
{
    ca_assert(is_string(left));

    char buf[64];
    sprintf(buf, "%d", value);
    string_append(left, buf);
}
void string_append_f(Value* left, float value)
{
    ca_assert(is_string(left));

    const int BUF_SIZE = 64;
    char buf[BUF_SIZE];
    sprintf(buf, "%f", value);

    // Chop off extra zeros.
    int dotPosition = -1;

    for (int i=0; i < BUF_SIZE; i++) {
        if (buf[i] == 0)
            break;

        if (buf[i] == '.') {
            dotPosition = i;
            break;
        }
    }

    for (size_t i = strlen(buf) - 1; i > dotPosition + 1; i--) {
        if (buf[i] == '0')
            buf[i] = 0;
        else
            break;
    }

    string_append(left, buf);
}
void string_append_char(Value* left, char c)
{
    char buf[2];
    buf[0] = c;
    buf[1] = 0;
    string_append(left, buf);
}
void string_append_ptr(Value* left, void* ptr)
{
    char buf[64];
    sprintf(buf, "%p", ptr);
    string_append(left, buf);
}

void blob_append(Value* left, Value* right)
{
    ca_assert(is_string(left));
    ca_assert(is_string(left));
}

void string_resize(Value* s, int length)
{
    if (length < 0)
        length = string_length(s) + length;

    StringData** data = (StringData**) &s->value_data.ptr;
    string_resize(data, length);
}

bool string_equals(Value* s, const char* str)
{
    if (is_string(s))
        return strcmp(as_cstring(s), str) == 0;

    // Deprecated method to compare to a symbol value.
    if (is_symbol(s)) {
        if (str[0] != ':')
            return false;

        return strcmp(symbol_as_string(s), str + 1) == 0;
    }

    return false;
}

bool string_empty(Value* s)
{
    return is_null(s) || string_equals(s, "");
}

bool string_starts_with(Value* s, const char* beginning)
{
    const char* left = as_cstring(s);
    for (int i=0;; i++) {
        if (beginning[i] == 0)
            return true;
        if (left[i] != beginning[i])
            return false;
    }
}

bool string_ends_with(Value* s, const char* ending)
{
    int ending_len = (int) strlen(ending);
    int s_len = string_length(s);
    const char* left = as_cstring(s);

    for (int i=0; i < ending_len; i++)
        if (left[s_len - ending_len + i] != ending[i])
            return false;

    return true;
}

void string_remove_suffix(Value* s, const char* str)
{
    if (!string_ends_with(s, str))
        return;

    string_resize(s, string_length(s) - int(strlen(str)));
}

char string_get(Value* s, int index)
{
    return as_cstring(s)[index];
}

int string_length(Value* s)
{
    return string_length((StringData*) s->value_data.ptr);
}

bool string_less_than(Value* left, Value* right)
{
    return strcmp(as_cstring(left), as_cstring(right)) < 0;
}

void string_prepend(Value* result, Value* prefix)
{
    Value output;
    copy(prefix, &output);
    string_append(&output, result);
    move(&output, result);
}
void string_prepend(Value* result, const char* prefix)
{
    Value output;
    set_string(&output, prefix);
    string_append(&output, result);
    move(&output, result);
}

void string_substr(Value* s, int start, int len, Value* out)
{
    if (s == out)
        internal_error("Usage error in string_substr, 's' cannot be 'out'");

    if (len == -1)
        len = string_length(s) - start;
    else if ((start + len) > string_length(s))
        len = string_length(s) - start;

    set_string(out, as_cstring(s) + start, len);
}

void string_slice(Value* s, int start, int end, Value* out)
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

void string_slice(Value* str, int start, int end)
{
    touch(str);

    if (end == -1)
        end = string_length(str);

    int newSize = end - start;

    if (start > 0)
        memmove((char*) as_cstring(str), as_cstring(str) + start, newSize);

    string_resize(str, newSize);
}

int string_find_char(Value* s, int start, char c)
{
    const char* cstr = as_cstring(s);

    for (int i=start; cstr[i] != 0; i++)
        if (cstr[i] == c)
            return i;
    return -1;
}

int string_find_char_from_end(Value* s, char c)
{
    const char* cstr = as_cstring(s);

    for (int i= string_length(s) - 1; i >= 0; i--)
        if (cstr[i] == c)
            return i;
    return -1;
}

void string_quote_and_escape(Value* s)
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

void string_unquote_and_unescape(Value* s)
{
    if (string_empty(s))
        return;

    const char* input = as_cstring(s);

    char quote = input[0];

    int quoteSize = 1;
    if (quote == '<')
        quoteSize = 3;

    int end = string_length(s) - quoteSize;

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

void string_join(Value* list, Value* separator, Value* out)
{
    set_string(out, "");
    for (int i=0; i < list_length(list); i++) {
        if (i != 0 && separator != NULL && !is_null(separator))
            string_append(out, separator);
        string_append(out, list_get(list, i));
    }
}

void string_split(Value* s, char sep, Value* listOut)
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

std::string as_string(Value* value)
{
    stat_increment(StringToStd);
    return std::string(as_cstring(value));
}

char* string_initialize(Value* value, int length)
{
    make(TYPES.string, value);
    StringData* data = string_create(length);
    value->value_data.ptr = data;
    return data->str;
}

void set_string(Value* value, const char* s)
{
    make(TYPES.string, value);
    int length = (int) strlen(s);
    StringData* data = string_create(length);
    memcpy(data->str, s, length + 1);
    value->value_data.ptr = data;
}

void set_string(Value* value, const char* s, int length)
{
    make(TYPES.string, value);
    StringData* data = string_create(length);
    memcpy(data->str, s, length);
    data->str[length] = 0;
    value->value_data.ptr = data;
}

char* set_blob(Value* value, int length)
{
    make(TYPES.string, value);
    StringData* data = string_create(length);
    memset(data->str, 0, length);
    value->value_data.ptr = data;
    return data->str;
}

char* circa_strdup(const char* s)
{
    size_t length = strlen(s);
    char* out = (char*) malloc(length + 1);
    memcpy(out, s, length + 1);
    return out;
}

char* as_blob(Value* value)
{
    return (char*) as_cstring(value);
}

void blob_append_char(Value* blob, char c)
{
    int size = string_length(blob);
    string_resize(blob, size + 1);
    as_blob(blob)[size] = c;
}

void blob_append_u8(Value* blob, u8 val)
{
    int size = string_length(blob);
    string_resize(blob, size + 1);
    as_blob(blob)[size] = val;
}

void blob_append_u16(Value* blob, u16 val)
{
    int size = string_length(blob);
    string_resize(blob, size + 2);
    u16* position = (u16*) &as_blob(blob)[size];
    *position = val;
}

void blob_append_u32(Value* blob, u32 val)
{
    int size = string_length(blob);
    string_resize(blob, size + 4);
    u32* position = (u32*) &as_blob(blob)[size];
    *position = val;
}
void blob_append_float(Value* blob, float f)
{
    int size = string_length(blob);
    string_resize(blob, size + 4);
    float* position = (float*) &as_blob(blob)[size];
    *position = f;
}

void blob_append_space(Value* blob, size_t additionalSize)
{
    size_t size = string_length(blob);
    string_resize(blob, int(size + additionalSize));
    memset(as_blob(blob) + size, 0, additionalSize);
}

char blob_read_char(const char* data, u32* pos)
{
    char c = data[*pos];
    *pos += 1;
    return c;
}

u8 blob_read_u8(const char* data, u32* pos)
{
    u8 c = data[*pos];
    *pos += 1;
    return c;
}

u16 blob_read_u16(const char* data, u32* pos)
{
    u16 value = *((u16*) &data[*pos]);
    *pos += 2;
    return value;
}

u32 blob_read_u32(const char* data, u32* pos)
{
    u32 value = *((u32*) &data[*pos]);
    *pos += 4;
    return value;
}

float blob_read_float(const char* data, u32* pos)
{
    float value = *((float*) &data[*pos]);
    *pos += 4;
    return value;
}

void* blob_read_pointer(const char* data, u32* pos)
{
    void* value = *((void**) &data[*pos]);
    *pos += sizeof(void*);
    return value;
}

void blob_write_u8(char* data, u32* pos, u8 value)
{
    *((u8*) &data[*pos]) = value;
    *pos += 1;
}

void blob_write_u32(char* data, u32* pos, u32 value)
{
    *((u32*) &data[*pos]) = value;
    *pos += 4;
}

void blob_write_pointer(char* data, u32* pos, void* value)
{
    *((void**) &data[*pos]) = value;
    *pos += sizeof(void*);
}

static char to_hex_digit(int i)
{
    if (i >= 0 && i < 10)
        return '0' + i;
    return 'a' + (i - 10);
}

void blob_to_hex_string(Value* blob, Value* str)
{
    set_string(str, "");

    for (int i=0; i < string_length(blob); i++) {
        char c = as_blob(blob)[i];

        string_append_char(str, to_hex_digit(c / 16));
        string_append_char(str, to_hex_digit(c % 16));
    }
}

CIRCA_EXPORT int circa_string_length(Value* string)
{
    return string_length(string);
}

} // namespace circa
