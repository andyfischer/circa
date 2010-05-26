// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "type.h"

namespace circa {
namespace callable_t {

    bool is_subtype(Type* type, Type* otherType)
    {
        return otherType == type_contents(FUNCTION_TYPE)
            || otherType == type_contents(TYPE_TYPE);
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->isSubtype = is_subtype;
    }
}
}
