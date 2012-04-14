// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "bool.h"

namespace circa {
namespace bool_t {
    void reset(Type*, caValue* value)
    {
        set_bool(value, false);
    }
    std::string to_string(caValue* value)
    {
        if (as_bool(value))
            return "true";
        else
            return "false";
    }
    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, bool_t::to_string(term_value(term)), term, TK_BOOL);
    }
    void setup_type(Type* type)
    {
        type->name = name_from_string("bool");
        type->storageType = STORAGE_TYPE_BOOL;
        type->reset = reset;
        type->toString = to_string;
        type->formatSource = format_source;
    }
} // namespace bool_t
} // namespace circa
