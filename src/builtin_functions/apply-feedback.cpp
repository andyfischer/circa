// Copyright 2008 Paul Hodge

#include <circa.h>

// This function is deprecated

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
            apply(&branch, targetsFunction.feedbackPropogateFunction,
                    RefList(target, desired));
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
        APPLY_FEEDBACK = import_function(kernel, evaluate, "apply_feedback(any,any)");
        as_function(APPLY_FEEDBACK).setInputMeta(0,true);
        as_function(APPLY_FEEDBACK).stateType = BRANCH_TYPE;
    }
}
}
