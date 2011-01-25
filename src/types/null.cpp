// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace null_t {

    std::string toString(TaggedValue* value)
    {
        return "null";
    }

    void setup_type(Type* type)
    {
        type->name = "null";
        type->toString = toString;
    }
}
}
