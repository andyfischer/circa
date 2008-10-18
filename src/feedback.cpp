// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtins.h"
#include "function.h"
#include "importing.h"
#include "runtime.h"

namespace circa {

namespace feedback {

    void apply_feedback(Term* caller)
    {
        Branch &branch = as_branch(caller->state);
        branch.clear();

        Term* target = caller->inputs[0];
        Term* desired = caller->inputs[1];

        Function& targetsFunction = as_function(target->function);

        if (targetsFunction.feedbackPropogateFunction != NULL)
        {
            apply_function(branch, targetsFunction.feedbackPropogateFunction,
                    ReferenceList(target, desired));
        } else {
            std::stringstream out;
            out << "Function " << targetsFunction.name <<
                " doesn't have a feedback-assign function";
            error_occured(caller, out.str());
            return;
        }

        evaluate_branch(branch);
    }
}

void initialize_feedback_functions(Branch& kernel)
{
    APPLY_FEEDBACK = import_c_function(kernel, feedback::apply_feedback,
            "function apply-feedback(any,any)");
    as_function(APPLY_FEEDBACK).meta = true;
    as_function(APPLY_FEEDBACK).stateType = BRANCH_TYPE;
}

} // namespace circa
