// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace errored_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(errored, "errored(any) -> bool")
    {
        set_bool(OUTPUT, is_error(INPUT(0)));
    }
    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        ERRORED_FUNC = kernel["errored"];
    }
}
}
