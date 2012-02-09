// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace void_t {
    std::string to_string(TValue*)
    {
        return "<void>";
    }
    void cast(CastResult* result, TValue* source, Type* type,
        TValue* dest, bool checkOnly)
    {
        if (!is_null(source)) {
            result->success = false;
            return;
        }

        if (checkOnly)
            return;

        set_null(dest);
    }
    void setup_type(Type* type)
    {
        type->name = "void";
        type->cast = cast;
        type->toString = to_string;
    }
} // namespace void_t
} // namespace circa
