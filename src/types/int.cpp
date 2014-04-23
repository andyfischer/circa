// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "./number.h"

namespace circa {
namespace int_t {
    void reset(Type*, caValue* v)
    {
        set_int(v, 0);
    }

    bool equals(caValue* a, caValue* b)
    {
        if (is_float(b))
            return number_t::equals(a, b);
        if (!is_int(b))
            return false;
        return as_int(a) == as_int(b);
    }
    int hashFunc(caValue* a)
    {
        return as_int(a);
    }
    void to_string(caValue* value, caValue* asStr)
    {
        string_append(asStr, as_int(value));
    }
    void setup_type(Type* type)
    {
        if (string_equals(&type->name, ""))
            set_string(&type->name, "int");
        type->storageType = sym_StorageTypeInt;
        type->reset = reset;
        type->equals = equals;
        type->hashFunc = hashFunc;
        type->toString = to_string;
    }
}
}
