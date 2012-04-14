// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace equals_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(equals, "equals(any,any) -> bool")
    {
        set_bool(OUTPUT, equals(INPUT(0), INPUT(1)));
    }

    CA_DEFINE_FUNCTION(not_equals, "not_equals(any,any) -> bool")
    {
        set_bool(OUTPUT, !equals(INPUT(0), INPUT(1)));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
