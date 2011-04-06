// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "type.h"

namespace circa {
namespace indexable_t {

    void staticTypeQuery(Type* type, StaticTypeQuery* query)
    {
        Type* subjectType = declared_type(query->subject);
        if (subjectType->getIndex != NULL
                && subjectType->numElements != NULL)
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
