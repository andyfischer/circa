// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "common.h"

namespace circa {
namespace opaque_pointer_t {

    void toString(caValue* val, caValue* out)
    {
        char buf[17];
        sprintf(buf, "%p", as_opaque_pointer(val));
        string_append(out, buf);
    }

    void setup_type(Type* type)
    {
        if (string_equals(&type->name, ""))
            set_string(&type->name, "opaque_pointer");
        type->storageType = sym_StorageTypeOpaquePointer;
        type->toString = toString;
        type->hashFunc = common_type_callbacks::shallow_hash_func;
    }

} // namespace opaque_pointer_t
} // namespace circa
