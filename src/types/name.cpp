// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace name_t {

    std::string to_string(caValue* value)
    {
        return std::string(":") + name_to_string(as_int(value));
    }
    void format_source(StyledSource* source, Term* term)
    {
        std::string s = name_t::to_string(term_value(term));
        append_phrase(source, s.c_str(), term, TK_NAME);
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = name_from_string("Name");
        type->storageType = STORAGE_TYPE_INT;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}
}
