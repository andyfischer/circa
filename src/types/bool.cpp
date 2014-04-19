// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "bool.h"

namespace circa {
namespace bool_t {
    void reset(Type*, caValue* value)
    {
        set_bool(value, false);
    }
    int hashFunc(caValue* a)
    {
        return as_bool(a) ? 1 : 0;
    }
    void to_string(caValue* value, caValue* out)
    {
        if (as_bool(value))
            string_append(out, "true");
        else
            string_append(out, "false");
    }
    void format_source(caValue* source, Term* term)
    {
        Value str;
        bool_t::to_string(term_value(term), &str);
        append_phrase(source, as_cstring(&str), term, tok_Bool);
    }
    void setup_type(Type* type)
    {
        set_string(&type->name, "bool");
        type->storageType = sym_StorageTypeBool;
        type->reset = reset;
        type->hashFunc = hashFunc;
        type->toString = to_string;
        type->formatSource = format_source;
    }
} // namespace bool_t
} // namespace circa
