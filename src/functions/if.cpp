// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace if_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(evaluate, "if(bool) -> any")
    {
#ifndef BYTECODE
        if (BOOL_INPUT(0))
            evaluate_branch(CONTEXT, CALLER->nestedContents);
#endif
    }

    CA_DEFINE_FUNCTION(join_function, "join() -> any")
    {
        // Compilation placeholder
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        IF_FUNC = kernel["if"];
        JOIN_FUNC = kernel["join"];
    }
}
}
