// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace symbol_t {

    std::string to_string(TaggedValue* value)
    {
        return std::string(":") + symbol_text(as_int(value));
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "symbol";
        type->storageType = STORAGE_TYPE_INT;
        type->toString = to_string;
    }
}
}
