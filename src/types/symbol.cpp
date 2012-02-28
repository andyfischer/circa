// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace symbol_t {

    std::string to_string(caValue* value)
    {
        return std::string(":") + name_to_string(as_int(value));
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = name_from_string("symbol");
        type->storageType = STORAGE_TYPE_INT;
        type->toString = to_string;
    }
}
}
