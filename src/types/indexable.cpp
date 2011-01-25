// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "type.h"

namespace circa {
namespace indexable_t {

    bool is_subtype(Type* type, Type* otherType)
    {
        return otherType->getIndex != NULL
            && otherType->numElements != NULL;
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->isSubtype = is_subtype;
    }
}
}
