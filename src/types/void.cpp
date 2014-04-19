// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace void_t {
    void to_string(caValue*, caValue* out)
    {
        string_append(out, "<void>");
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
        set_string(&type->name, "void");
        type->cast = cast;
        type->toString = to_string;
    }
} // namespace void_t
} // namespace circa
