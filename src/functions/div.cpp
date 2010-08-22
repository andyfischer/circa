// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace div_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(div_f, "div_f(number,number) -> number")
    {
        make_float(OUTPUT, FLOAT_INPUT(0) / FLOAT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(div_i, "div_i(int,int) -> int")
    {
        make_int(OUTPUT, INT_INPUT(0) / INT_INPUT(1));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        DIV_FUNC = create_overloaded_function(kernel, "div", RefList(kernel["div_f"]));
    }
}
} // namespace circa
