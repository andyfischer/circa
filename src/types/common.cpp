// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "tagged_value.h"
#include "type.h"

namespace circa {
namespace common_type_callbacks {

    int shallow_hash_func(TaggedValue* value)
    {
        return value->value_data.asint;
    }

}
}

