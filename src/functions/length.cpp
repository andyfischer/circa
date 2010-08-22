// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace length_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(length,  "length(List) -> int;"
            "'Return the number of items in the given list' end")
    {
        make_int(OUTPUT, num_elements(INPUT(0)));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
