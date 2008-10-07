// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "builtins.h"
#include "function.h"

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

        }
    }
}

void initialize_feedback_functions(Branch* kernel)
{

}

} // namespace circa
