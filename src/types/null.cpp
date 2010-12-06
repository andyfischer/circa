// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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
