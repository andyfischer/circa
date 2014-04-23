// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace number_t {
    void reset(Type*, caValue* value)
    {
        set_float(value, 0);
    }
    void cast(CastResult* result, caValue* value, Type* type, bool checkOnly)
    {
        if (!(is_int(value) || is_float(value))) {
            result->success = false;
            return;
        }

        if (checkOnly)
            return;

        set_float(value, to_float(value));
    }

    bool equals(caValue* a, caValue* b)
    {
        if (!is_float(b) && !is_int(b))
            return false;
        return to_float(a) == to_float(b);
    }
    void to_string(caValue* value, caValue* out)
    {
        string_append_f(out, as_float(value));
    }
    void staticTypeQuery(Type* type, StaticTypeQuery* query)
    {
        if (query->subjectType == TYPES.float_type || query->subjectType == TYPES.int_type)
            query->succeed();
        else
            query->fail();
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        set_string(&type->name, "number");
        type->storageType = sym_StorageTypeFloat;
        type->reset = reset;
        type->cast = cast;
        type->equals = equals;
        type->staticTypeQuery = staticTypeQuery;
        type->toString = to_string;
    }
}
}
