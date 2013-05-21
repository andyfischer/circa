// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace mult_function {

    CA_FUNCTION(evaluate_f)
    {
        float product = circa_float_input(STACK, 0) * circa_float_input(STACK, 1);
        set_float(circa_output(STACK, 0), product);
    }

    CA_FUNCTION(evaluate_i)
    {
        int product = circa_int_input(STACK, 0) * circa_int_input(STACK, 1);
        set_int(circa_output(STACK, 0), product);
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate_i, "mult_i(int a,int b) -> int");
        import_function(kernel, evaluate_f, "mult_f(number a,number b) -> number");
    }
}
} // namespace circa
