// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace finish_minor_branch_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(finish_minor_branch_func, "finish_minor_branch()")
    {
        Branch& contents = CALLER->nestedContents;
        start_using(contents);
        for (int i=0; i < contents.length(); i++)
            evaluate_single_term(CONTEXT, contents[i]);
        finish_using(contents);
        set_null(OUTPUT);
    }

    void postCompile(Term* finishBranchTerm)
    {
        Branch& contents = finishBranchTerm->nestedContents;
        contents.clear();

        Branch& outerContents = *finishBranchTerm->owningBranch;

        // Find every state var that was opened in this branch, and add a
        // preserve_state_result() call for each.
        for (int i=0; i < outerContents.length(); i++) {
            Term* term = outerContents[i];

            if (term == NULL)
                continue;

            if (term->function == GET_STATE_FIELD_FUNC) {
                if (term->name == "")
                    continue;
                Term* outcome = get_named_at(finishBranchTerm, term->name);
                apply(contents, PRESERVE_STATE_RESULT_FUNC, RefList(outcome));
            }
        }
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        FINISH_MINOR_BRANCH_FUNC = kernel["finish_minor_branch"];
        get_function_attrs(FINISH_MINOR_BRANCH_FUNC)->postCompile = postCompile;
    }
}
}
