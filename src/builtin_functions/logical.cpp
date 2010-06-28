// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace logical_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(and, "and(bool a, bool b) -> bool;"
                "'Return whether a and b are both true' end")
    {
        set_bool(OUTPUT, as_bool(INPUT(0)) && as_bool(INPUT(1)));
    }

    CA_DEFINE_FUNCTION(or, "or(bool a, bool b) -> bool;"
                "'Return whether a or b are both true' end")
    {
        set_bool(OUTPUT, as_bool(INPUT(0)) || as_bool(INPUT(1)));
    }

    CA_DEFINE_FUNCTION(not, "not(bool) -> bool")
    {
        set_bool(OUTPUT, !BOOL_INPUT(0));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
