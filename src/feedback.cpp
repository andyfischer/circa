// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtins.h"
#include "evaluation.h"
#include "function.h"
#include "importing.h"
#include "operations.h"

namespace circa {

namespace feedback {

    void apply_feedback(Term* caller)
    {
        Branch &branch = as_branch(caller->state);
        branch.clear();

        Term* target = caller->inputs[0];
        Term* desired = caller->inputs[1];

        Function& targetsFunction = as_function(target->function);

        if (targetsFunction.feedbackAssignFunction != NULL)
        {
            apply_function(branch, targetsFunction.feedbackAssignFunction,
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
    Term* apply_feedback = import_c_function(kernel, feedback::apply_feedback,
            "function apply-feedback(any,any)");
    as_function(apply_feedback).meta = true;
    as_function(apply_feedback).stateType = BRANCH_TYPE;
}

} // namespace circa
