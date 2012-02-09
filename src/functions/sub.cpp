// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace sub_function {

    CA_FUNCTION(evaluate_i)
    {
        set_int(OUTPUT, INT_INPUT(0) - INT_INPUT(1));
    }

    CA_FUNCTION(evaluate_f)
    {
        set_float(OUTPUT, FLOAT_INPUT(0) - FLOAT_INPUT(1));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate_i, "sub_i(int,int) -> int");
        import_function(kernel, evaluate_f, "sub_f(number,number) -> number");
    }
}
} // namespace circa
