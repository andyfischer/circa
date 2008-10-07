// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtins.h"
#include "evaluation.h"
#include "function.h"
#include "operations.h"

namespace circa {

namespace feedback {

    void apply_feedback(Term* caller)
    {
        Branch &branch = as_branch(caller->state);
        branch.clear();

        Term* target = caller->inputs[0];
        Term* desired = caller->inputs[1];

        Function& targetsFunction = as_function(target);

        if (targetsFunction.feedbackAssignFunction != NULL)
        {
            apply_function(branch, targetsFunction.feedbackAssignFunction,
                    ReferenceList(target, desired));
        }

        evaluate_branch(branch);
    }
}

void initialize_feedback_functions(Branch* kernel)
{

}

} // namespace circa
