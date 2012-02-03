// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa_internal.h"
#include "importing_macros.h"

namespace circa {
namespace div_function {

    CA_FUNCTION(div_f)
    {
        set_float(OUTPUT, FLOAT_INPUT(0) / FLOAT_INPUT(1));
    }

    CA_FUNCTION(div_i)
    {
        set_int(OUTPUT, INT_INPUT(0) / INT_INPUT(1));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, div_f, "div_f(number,number) -> number");
        DIV_FUNC = import_function(kernel, div_f, "div(number,number) -> number");
        import_function(kernel, div_i, "div_i(int,int) -> int");
    }
}
} // namespace circa
