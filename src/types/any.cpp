// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace any_t {

    std::string to_string(TaggedValue*)
    {
        return "<any>";
    }
    bool matches_type(Type*, Type*)
    {
        return true;
    }
    void cast(CastResult* result, TaggedValue* source, Type* type,
        TaggedValue* dest, bool checkOnly)
    {
        // casting to 'any' always succeeds.
        if (checkOnly)
            return;

        copy(source, dest);
    }

} // namespace any_t
} // namespace circa
