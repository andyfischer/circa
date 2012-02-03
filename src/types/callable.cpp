// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "kernel.h"
#include "type.h"
#include "token.h"

namespace circa {
namespace callable_t {

    void staticTypeQuery(Type* type, StaticTypeQuery* query)
    {
        if (declared_type(query->subject) == unbox_type(FUNCTION_TYPE)
                || declared_type(query->subject) == unbox_type(TYPE_TYPE))
            query->succeed();
        else
            query->fail();
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->staticTypeQuery = staticTypeQuery;
    }
}
}
