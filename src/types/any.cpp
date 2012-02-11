// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace any_t {

    std::string to_string(TValue*)
    {
        return "<any>";
    }
    void staticTypeQuery(Type*, StaticTypeQuery* query)
    {
        return query->succeed();
    }
    void cast(CastResult* result, TValue* source, Type* type,
        TValue* dest, bool checkOnly)
    {
        // casting to 'any' always succeeds.
        if (checkOnly)
            return;

        copy(source, dest);
    }
    void setup_type(Type* type)
    {
        type->name = string_to_symbol("any");
        type->toString = to_string;
        type->staticTypeQuery = staticTypeQuery;
        type->cast = cast;
    }

} // namespace any_t
} // namespace circa
