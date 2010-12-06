// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

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

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
