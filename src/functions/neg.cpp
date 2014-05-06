// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace neg_function  {

    void evaluate_f(caStack* stack)
    {
        set_float(circa_output(stack, 0), -circa_float_input(stack, 0));
    }

    void evaluate_i(caStack* stack)
    {
        set_int(circa_output(stack, 0), -circa_int_input(stack, 0));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate_i, "neg_i(int i) -> int");
        import_function(kernel, evaluate_f, "neg_f(number n) -> number");
    }
}
}
