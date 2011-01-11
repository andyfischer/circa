// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace finish_minor_branch_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(finish_minor_branch_func, "finish_minor_branch()")
    {
        Branch& contents = CALLER->nestedContents;
        for (int i=0; i < contents.length(); i++)
            evaluate_single_term(CONTEXT, contents[i]);
    }

    void postCompile(Term* term)
    {
        Term* sub = find_enclosing_subroutine(term);
        if (sub == NULL)
            return;
        update_subroutine_return_contents(sub, term);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        FINISH_MINOR_BRANCH_FUNC = kernel["finish_minor_branch"];
        get_function_attrs(FINISH_MINOR_BRANCH_FUNC)->postCompile = postCompile;
    }
}
}
