// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace unknown_function_function {

    void setup(Branch* kernel)
    {
        FUNCS.unknown_function = import_function(kernel, NULL,
            "unknown_function(any...) -> any");
    }
}
}
