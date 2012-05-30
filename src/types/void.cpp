// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace void_t {
    std::string to_string(caValue*)
    {
        return "<void>";
    }
    void cast(CastResult* result, caValue* value, Type* type, bool checkOnly)
    {
        if (!is_null(value)) {
            result->success = false;
            return;
        }
    }
    void setup_type(Type* type)
    {
        type->name = name_from_string("void");
        type->cast = cast;
        type->toString = to_string;
    }
} // namespace void_t
} // namespace circa
