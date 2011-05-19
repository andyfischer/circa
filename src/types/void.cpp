// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace void_t {
    std::string to_string(Value*)
    {
        return "<void>";
    }
    void cast(CastResult* result, Value* source, Type* type,
        Value* dest, bool checkOnly)
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
