// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace null_t {

    std::string toString(caValue* value)
    {
        return "null";
    }
    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, "null", term, tok_Null);
    }

    caValue* getField(caValue* value, const char* field)
    {
        return NULL;
    }

    void setup_type(Type* type)
    {
        type->name = name_from_string("null");
        type->toString = toString;
        type->formatSource = formatSource;
        type->getField = getField;
    }
}
}
