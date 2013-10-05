// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace div_function {

    void div_f(caStack* stack)
    {
        set_float(circa_output(stack, 0), circa_float_input(stack, 0) / circa_float_input(stack, 1));
    }

    void div_i(caStack* stack)
    {
        int a = to_int(circa_input(stack, 0));
        int b = to_int(circa_input(stack, 1));
        set_int(circa_output(stack, 0), a / b);
    }

    void setup(Block* kernel)
    {
        import_function(kernel, div_f, "div_f(number a,number b) -> number");
        FUNCS.div = import_function(kernel, div_f, "div(number a,number b) -> number");
        FUNCS.div_i = import_function(kernel, div_i, "div_i(number a,number b) -> int");
    }
}
} // namespace circa
