// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa_internal.h"

namespace circa {
namespace unknown_type_function {

    CA_FUNCTION(evaluate)
    {
    }

    void setup(Branch* kernel)
    {
        UNKNOWN_TYPE_FUNC = import_function(kernel, evaluate, "unknown_type() -> Type");
    }
}
}
