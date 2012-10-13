// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace abs_function {

    void abs(caStack* stack)
    {
        set_float(circa_output(stack, 0), std::abs(circa_float_input(stack, 0)));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, abs, "abs(number n) -> number 'Absolute value'");
    }
}
}
