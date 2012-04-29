// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "ref.h"

namespace circa {
namespace ref_t {

    void setup_type(Type* type)
    {
        type->name = name_from_string("Term");
        type->storageType = STORAGE_TYPE_REF;
    }
}

} // namespace circa
