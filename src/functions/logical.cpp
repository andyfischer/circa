// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace logical_function {

    void and_func(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
            circa_bool_input(stack, 0) && circa_bool_input(stack, 1));
    }

    void or_func(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
            circa_bool_input(stack, 0) || circa_bool_input(stack, 1));
    }

    void not_func(caStack* stack)
    {
        set_bool(circa_output(stack, 0), !circa_bool_input(stack, 0));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, and_func, "and(bool a, bool b) -> bool;"
                "'Return whether a and b are both true'");
        import_function(kernel, or_func, "or(bool a, bool b) -> bool;"
                "'Return whether a or b are both true'");
        import_function(kernel, not_func, "not(bool b) -> bool");
    }
}
}
