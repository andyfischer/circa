// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace abs_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(abs, "abs(number n) -> number 'Absolute value'")
    {
        set_float(OUTPUT, std::abs(FLOAT_INPUT(0)));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
