// Copyright (c) 2007-2010 Andrew Fischer. All rights reserved

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace increment_function {

    void increment(caStack* stack)
    {
        set_int(circa_output(stack, 0), circa_int_input(stack, 0) + 1);
    }

    void decrement(caStack* stack)
    {
        set_int(circa_output(stack, 0), circa_int_input(stack, 0) - 1);
    }

    void setup(Block* kernel)
    {
        import_function(kernel, increment, "increment(int i) -> int");
        import_function(kernel, decrement, "decrement(int i) -> int");
    }
}
}
