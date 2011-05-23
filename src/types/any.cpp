// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "type.h"

namespace circa {
namespace any_t {

    std::string to_string(TaggedValue*)
    {
        return "<any>";
    }
    void staticTypeQuery(Type*, StaticTypeQuery* query)
    {
        return query->succeed();
    }
    void cast(CastResult* result, TaggedValue* source, Type* type,
        TaggedValue* dest, bool checkOnly)
    {
        // casting to 'any' always succeeds.
        if (checkOnly)
            return;

        copy(source, dest);
    }
    void setup_type(Type* type)
    {
        type->name = "any";
        type->toString = to_string;
        type->staticTypeQuery = staticTypeQuery;
        type->cast = cast;
    }

} // namespace any_t
} // namespace circa
