// Copyright (c) 2007-2010 Andrew Fischer. All rights reserved

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace increment_function {
    
    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(increment, "increment(int i) -> int")
    {
        set_int(OUTPUT, INT_INPUT(0) + 1);
    }

    CA_DEFINE_FUNCTION(decrement, "decrement(int i) -> int")
    {
        set_int(OUTPUT, INT_INPUT(0) - 1);
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
