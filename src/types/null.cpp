// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace null_t {

    void toString(caValue* value, caValue* out)
    {
        string_append(out, "null");
    }
    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, "null", term, tok_Null);
    }

    void setup_type(Type* type)
    {
        set_string(&type->name, "null");
        type->toString = toString;
        type->formatSource = formatSource;
    }
}
}
