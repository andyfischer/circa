// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace equals_function {

    void equals_func(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                equals(circa_input(stack, 0), circa_input(stack, 1)));
    }

    void not_equals(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                !equals(circa_input(stack, 0), circa_input(stack, 1)));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, equals_func, "equals(any a,any b) -> bool");
        import_function(kernel, not_equals, "not_equals(any a,any b) -> bool");
    }
}
}
