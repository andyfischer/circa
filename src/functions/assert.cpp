// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>
#include "importing_macros.h"

namespace circa {
namespace assert_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(assert, "assert(bool condition);"
            "'Raises a runtime error if condition is false' end")
    {
        if (!BOOL_INPUT(0))
            error_occurred(CONTEXT, CALLER, "Assert failed");
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
