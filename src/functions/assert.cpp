// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace assert_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(assert, "assert(bool condition);"
            "'Raises a runtime error if condition is false'")
    {
        if (!BOOL_INPUT(0))
            RAISE_ERROR("Assert failed");
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
