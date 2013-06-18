// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace sub_function {

    void evaluate_i(caStack* stack)
    {
        set_int(circa_output(stack, 0), circa_int_input(stack, 0) - circa_int_input(stack, 1));
    }

    void evaluate_f(caStack* stack)
    {
        set_float(circa_output(stack, 0), circa_float_input(stack, 0) - circa_float_input(stack, 1));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate_i, "sub_i(int a,int b) -> int");
        import_function(kernel, evaluate_f, "sub_f(number a,number b) -> number");
    }
}
} // namespace circa
