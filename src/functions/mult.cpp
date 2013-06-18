// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace mult_function {

    void evaluate_f(caStack* stack)
    {
        float product = circa_float_input(stack, 0) * circa_float_input(stack, 1);
        set_float(circa_output(stack, 0), product);
    }

    void evaluate_i(caStack* stack)
    {
        int product = circa_int_input(stack, 0) * circa_int_input(stack, 1);
        set_int(circa_output(stack, 0), product);
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate_i, "mult_i(int a,int b) -> int");
        import_function(kernel, evaluate_f, "mult_f(number a,number b) -> number");
    }
}
} // namespace circa
