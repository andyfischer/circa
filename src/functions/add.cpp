// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace add_function {

    void add_i_evaluate(caStack* stack)
    {
        int sum = circa_int_input(stack, 0) + circa_int_input(stack, 1);
        set_int(circa_output(stack, 0), sum);
    }

    void add_f_evaluate(caStack* stack)
    {
        float sum = circa_float_input(stack, 0) + circa_float_input(stack, 1);
        set_float(circa_output(stack, 0), sum);
    }

    void setup(Block* kernel)
    {
        FUNCS.add_i = import_function(kernel, add_i_evaluate, "add_i(int a, int b) -> int");
        FUNCS.add_f = import_function(kernel, add_f_evaluate, "add_f(number a, number b) -> number");
    }
} // namespace add_function
} // namespace circa
