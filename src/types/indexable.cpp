// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

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
    void cast(CastResult* result, caValue* value, Type* type, bool checkOnly)
    {
        // For now, always succeed
        result->success = true;
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->staticTypeQuery = staticTypeQuery;
        type->cast = cast;
    }
}
}
