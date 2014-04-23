// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace null_t {

    void toString(caValue* value, caValue* out)
    {
        string_append(out, "null");
    }
    void setup_type(Type* type)
    {
        set_string(&type->name, "null");
        type->toString = toString;
    }
}
}
