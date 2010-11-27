// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace bool_t {
    void reset(TaggedValue* value)
    {
        set_bool(value, false);
    }
    std::string to_string(TaggedValue* value)
    {
        if (as_bool(value))
            return "true";
        else
            return "false";
    }
    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, bool_t::to_string(term), term, token::BOOL);
    }
    void setup_type(Type* type)
    {
        type->name = "bool";
        type->reset = reset;
        type->toString = to_string;
        type->formatSource = format_source;
    }
} // namespace bool_t
} // namespace circa
