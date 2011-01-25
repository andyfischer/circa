// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace length_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(length,  "length(List) -> int;"
            "'Return the number of items in the given list'")
    {
        set_int(OUTPUT, num_elements(INPUT(0)));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
