// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace swap_function {

    void swap(caStack* stack)
    {
        copy(circa_input(stack, 0), circa_output(stack, 2));
        copy(circa_input(stack, 1), circa_output(stack, 1));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, swap, "swap(any :out, any :out)");
    }
}
}
