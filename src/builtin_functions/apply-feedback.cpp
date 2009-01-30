// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace apply_feedback_function {

    void evaluate(Term* caller)
    {
        Branch &branch = as_branch(caller->state);
        branch.clear();

        Term* target = caller->input(0);
        Term* desired = caller->input(1);

        Function& targetsFunction = as_function(target->function);

        if (targetsFunction.feedbackPropogateFunction != NULL)
        {
            apply_function(&branch, targetsFunction.feedbackPropogateFunction,
                    ReferenceList(target, desired));
        } else {
            std::stringstream out;
            out << "Function " << targetsFunction.name <<
                " doesn't have a feedback-propogate function";
            error_occured(caller, out.str());
            return;
        }

        evaluate_branch(branch);
    }

    void setup(Branch& kernel)
    {
        APPLY_FEEDBACK = import_function(kernel, evaluate,
                "function apply-feedback(any,any)");
        as_function(APPLY_FEEDBACK).setInputMeta(0,true);
        as_function(APPLY_FEEDBACK).stateType = BRANCH_TYPE;
    }
}
}
