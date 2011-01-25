// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace if_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(evaluate, "if(bool) -> any")
    {
        // Compilation placeholder
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        IF_FUNC = kernel["if"];
        JOIN_FUNC = kernel["join"];
    }
}
}
