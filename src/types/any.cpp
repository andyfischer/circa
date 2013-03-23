// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace any_t {

    std::string to_string(caValue*)
    {
        return "<any>";
    }
    void staticTypeQuery(Type*, StaticTypeQuery* query)
    {
        return query->succeed();
    }
    void cast(CastResult* result, caValue* source, Type* type, bool checkOnly)
    {
        // casting to 'any' always succeeds.
        result->success = true;
    }
    void setup_type(Type* type)
    {
        set_string(&type->name, "any");
        type->toString = to_string;
        type->staticTypeQuery = staticTypeQuery;
        type->cast = cast;
    }

} // namespace any_t
} // namespace circa
