// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace div_function {

    CA_FUNCTION(div_f)
    {
        set_float(OUTPUT, FLOAT_INPUT(0) / FLOAT_INPUT(1));
    }

    CA_FUNCTION(div_i)
    {
        int a = to_int(circa_input(_stack, 0));
        int b = to_int(circa_input(_stack, 1));
        set_int(circa_output(_stack, 0), a / b);
    }

    void setup(Block* kernel)
    {
        import_function(kernel, div_f, "div_f(number a,number b) -> number");
        FUNCS.div = import_function(kernel, div_f, "div(number a,number b) -> number");
        import_function(kernel, div_i, "div_i(number a,number b) -> int");
    }
}
} // namespace circa
