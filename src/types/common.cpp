// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace common_type_callbacks {

    int shallow_hash_func(TValue* value)
    {
        return value->value_data.asint;
    }

}
}

