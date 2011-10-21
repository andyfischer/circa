// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include "importing_macros.h"

namespace circa {
namespace assert_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(assert, "assert(bool condition);"
            "'Raises a runtime error if condition is false'")
    {
        if (!BOOL_INPUT(0))
            error_occurred(CONTEXT, CALLER, "Assert failed");
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
