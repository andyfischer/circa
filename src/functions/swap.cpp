// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace swap_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(swap_func, "swap(any :out, any :out)")
    {
        copy(INPUT(0), EXTRA_OUTPUT(1));
        copy(INPUT(1), EXTRA_OUTPUT(0));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
