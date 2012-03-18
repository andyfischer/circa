// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

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

std::string& as_std_string(caValue* value);

#if 0
namespace new_string_t {

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
StringData* create_str(int length)
{
    ca_assert(length > 0);
    StringData* result = (StringData*) malloc(sizeof(StringData) + length);
    result->refCount = 1;
    result->length = length;
    result->str[0] = 0;
    return result;
}

// Creates a duplicate of 'original'. Starts off with 1 ref.
StringData* duplicate(StringData* original)
{
    StringData* dup = create_str(original->length);
#ifdef WINDOWS
    strncpy_s(original->str, original->length, dup->str, dup->length);
#else
    strncpy(original->str, dup->str, dup->length);
#endif
    return dup;
}

// Return a version of this string which is safe to modify. If this data has
// multiple references, we'll return a new copy (and decref the original).
StringData* touch(StringData* original)
{
    ca_assert(original->refCount > 0);
    if (original->refCount == 1)
        return original;
    StringData* dup = duplicate(original);
    decref(original);
    return dup;
}

} // namespace string_t

#endif

namespace string_t {

    void initialize(Type* type, caValue* value)
    {
        set_pointer(value, new std::string());
    }
    void release(caValue* value)
    {
        delete ((std::string*) get_pointer(value));
        set_pointer(value, NULL);
    }

    void copy(Type* type, caValue* source, caValue* dest)
    {
        create(type, dest);
        *((std::string*) dest->value_data.ptr) = as_string(source);
    }
    void reset(caValue* v)
    {
        set_string(v, "");
    }
    int hashFunc(caValue* v)
    {
        const char* str = as_cstring(v);

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

    bool equals(Type*, caValue* lhs, caValue* rhs)
    {
        if (!is_string(rhs)) return false;
        return as_string(lhs) == as_string(rhs);
    }

    std::string to_string(caValue* value)
    {
        std::stringstream result;
        result << "'" << as_string(value) << "'";
        return result.str();
    }

    void format_source(StyledSource* source, Term* term)
    {
        if (term->hasProperty("syntax:originalString")) {
            append_phrase(source, term->stringProp("syntax:originalString"),
                    term, TK_STRING);
            return;
        }

        std::string quoteType = term->stringPropOptional("syntax:quoteType", "'");
        std::string result;
        if (quoteType == "<")
            result = "<<<" + as_string(term) + ">>>";
        else
            result = quoteType + as_string(term) + quoteType;

        append_phrase(source, result, term, TK_STRING);
    }
}

void string_setup_type(Type* type)
{
    reset_type(type);
    type->name = name_from_string("string");
    type->storageType = STORAGE_TYPE_STRING;
    type->initialize = string_t::initialize;
    type->release = string_t::release;
    type->copy = string_t::copy;
    type->equals = string_t::equals;
    type->hashFunc = string_t::hashFunc;
    type->toString = string_t::to_string;
    type->formatSource = string_t::format_source;
}

void string_append(caValue* left, caValue* right)
{
    if (is_string(right))
        as_std_string(left) += as_string(right);
    else
        as_std_string(left) += to_string(right);
}
void string_append(caValue* left, const char* right)
{
    as_std_string(left) += right;
}
void string_resize(caValue* s, int length)
{
    if (length < 0)
        length = as_std_string(s).size() + length;

    as_std_string(s).resize(length, ' ');
}
bool string_eq(caValue* s, const char* str)
{
    return as_std_string(s) == str;
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
    std::string& str = as_std_string(s);
    int len = str.size();

    for (int i=0; i < ending_len; i++)
        if (str[len - ending_len + i] != ending[i])
            return false;

    return true;
}
char string_get(caValue* s, int index)
{
    return as_cstring(s)[index];
}

int string_length(caValue* s)
{
    return strlen(as_cstring(s));
}

void string_slice(caValue* s, int start, int end, caValue* out)
{
    if (s == out)
        internal_error("Usage error in string_slice, 's' cannot be 'out'");

    if (end == -1)
        end = string_length(s);

    set_string(out, "");
    int len = end - start;
    string_resize(out, len);

    for (int i=0; i < len; i++)
        as_std_string(out)[i] = string_get(s, i + start);
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

std::string& as_std_string(caValue* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_STRING);
    return *((std::string*) value->value_data.ptr);
}

std::string const& as_string(caValue* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_STRING);
    return *((std::string*) value->value_data.ptr);
}

const char* as_cstring(caValue* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_STRING);
    return ((std::string*) value->value_data.ptr)->c_str();
}

void set_string(caValue* value, const char* s, int length)
{
    create(&STRING_T, value);
    as_std_string(value).assign(s, length);
}

} // namespace circa
