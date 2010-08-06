// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace return_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(return_func, "return(any)")
    {
        // Find the enclosing subroutine
        Term* parent = get_parent_term(CALLER);
        while (parent != NULL && !is_subroutine(parent))
            parent = get_parent_term(parent);

        if (parent == NULL)
            internal_error("return() couldn't find enclosing subroutine");

        CONTEXT->interruptSubroutine = true;
        copy(INPUT(0), &CONTEXT->subroutineOutput);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        RETURN_FUNC = kernel["return"];
    }
}
}
