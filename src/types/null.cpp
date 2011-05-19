// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace null_t {

    std::string toString(Value* value)
    {
        return "null";
    }
    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "null", term, token::NULL_TOKEN);
    }

    void setup_type(Type* type)
    {
        type->name = "null";
        type->toString = toString;
        type->formatSource = formatSource;
    }
}
}
