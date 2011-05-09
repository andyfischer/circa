// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "builtins.h"
#include "importing.h"
#include "errors.h"
#include "evaluation.h"
#include "source_repro.h"
#include "string_t.h"
#include "tagged_value.h"
#include "token.h"
#include "type.h"

namespace circa {

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

    void initialize(Type* type, TaggedValue* value)
    {
        set_pointer(value, new std::string());
    }
    void release(Type*, TaggedValue* value)
    {
        delete ((std::string*) get_pointer(value));
        set_pointer(value, NULL);
    }

    void copy(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        change_type(dest, type);
        *((std::string*) dest->value_data.ptr) = as_string(source);
    }
    void reset(TaggedValue* v)
    {
        set_string(v, "");
    }
    int hashFunc(TaggedValue* v)
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

    bool equals(Type*, TaggedValue* lhs, TaggedValue* rhs)
    {
        if (!is_string(rhs)) return false;
        return as_string(lhs) == as_string(rhs);
    }

    std::string to_string(TaggedValue* value)
    {
        std::stringstream result;
        result << "'" << as_string(value) << "'";
        return result.str();
    }

    void format_source(StyledSource* source, Term* term)
    {
        if (term->hasProperty("syntax:originalString")) {
            append_phrase(source, term->stringProp("syntax:originalString"),
                    term, token::STRING);
            return;
        }

        std::string quoteType = term->stringPropOptional("syntax:quoteType", "'");
        std::string result;
        if (quoteType == "<")
            result = "<<<" + as_string(term) + ">>>";
        else
            result = quoteType + as_string(term) + quoteType;

        append_phrase(source, result, term, token::STRING);
    }

    CA_FUNCTION(length)
    {
        set_int(OUTPUT, int(INPUT(0)->asString().length()));
    }

    CA_FUNCTION(substr)
    {
        int start = INT_INPUT(1);
        int end = INT_INPUT(2);
        std::string const& s = as_string(INPUT(0));

        if (start < 0) return error_occurred(CONTEXT, CALLER, "Negative index");
        if (end < 0) return error_occurred(CONTEXT, CALLER, "Negative index");

        if ((unsigned) start > s.length()) {
            std::stringstream msg;
            msg << "Start index is too high: " << start;
            return error_occurred(CONTEXT, CALLER, msg.str().c_str());
        }
        if ((unsigned) (start+end) > s.length()) {
            std::stringstream msg;
            msg << "End index is too high: " << start;
            return error_occurred(CONTEXT, CALLER, msg.str().c_str());
        }

        set_string(OUTPUT, s.substr(start, end));
    }

    CA_FUNCTION(slice)
    {
        int start = INT_INPUT(1);
        int end = INT_INPUT(2);
        std::string const& s = as_string(INPUT(0));

        // Negative indexes are relatve to end of string
        if (start < 0) start = s.length() + start;
        if (end < 0) end = s.length() + end;

        if (start < 0) return set_string(OUTPUT, "");
        if (end < 0) return set_string(OUTPUT, "");

        if ((unsigned) start > s.length())
            start = s.length();

        if ((unsigned) end > s.length())
            end = s.length();

        if (end < start)
            return set_string(OUTPUT, "");

        set_string(OUTPUT, s.substr(start, end - start));
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "string";
        type->storageType = STORAGE_TYPE_STRING;
        type->initialize = initialize;
        type->release = release;
        type->copy = copy;
        type->equals = equals;
        type->hashFunc = hashFunc;
        type->toString = to_string;
        type->formatSource = format_source;
    }

    void postponed_setup_type(Type* type)
    {
        import_member_function(type, length, "length(string) -> int");
        import_member_function(type, substr, "substr(string,int,int) -> string");
        import_member_function(type, slice,  "slice(string,int,int) -> string");
    }
}

} // namespace circa
