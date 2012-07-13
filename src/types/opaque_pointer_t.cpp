// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "common.h"

namespace circa {
namespace opaque_pointer_t {

    std::string toString(caValue* val)
    {
        char buf[17];
        sprintf(buf, "%p", as_opaque_pointer(val));
        return buf;
    }

    void setup_type(Type* type)
    {
        if (type->name == name_None)
            type->name = name_from_string("opaque_pointer");
        type->storageType = STORAGE_TYPE_OPAQUE_POINTER;
        type->toString = toString;
        type->hashFunc = common_type_callbacks::shallow_hash_func;
    }

} // namespace opaque_pointer_t
} // namespace circa
