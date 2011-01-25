// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "type.h"

namespace circa {
namespace callable_t {

    bool is_subtype(Type* type, Type* otherType)
    {
        return otherType == unbox_type(FUNCTION_TYPE)
            || otherType == unbox_type(TYPE_TYPE);
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->isSubtype = is_subtype;
    }
}
}
