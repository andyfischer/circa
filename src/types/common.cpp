// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace common_type_callbacks {

    int shallow_hash_func(caValue* value)
    {
        return value->value_data.asint;
    }

}
}

