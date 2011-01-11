// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace return_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(return_func, "return(any)")
    {
        CONTEXT->interruptSubroutine = true;
        copy(INPUT(0), &CONTEXT->subroutineOutput);
    }

    void returnPostCompile(Term* term)
    {
        Term* sub = find_enclosing_subroutine(term);
        if (sub == NULL)
            return;
        update_subroutine_return_contents(sub, term);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        RETURN_FUNC = kernel["return"];
        get_function_attrs(RETURN_FUNC)->postCompile = returnPostCompile;
    }
}
}
