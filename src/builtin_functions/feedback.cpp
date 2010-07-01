// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace feedback_function {

    CA_FUNCTION(evaluate)
    {
        // No-op
    }

    void evaluate_apply_feedback(EvalContext*, Term* caller)
    {
        Branch& input = caller->input(0)->asBranch();
        refresh_training_branch(input, as_branch(caller));
    }

    void setup(Branch& kernel)
    {
        FEEDBACK_FUNC = import_function(kernel, evaluate, "feedback(any,any)");
        function_t::set_input_meta(FEEDBACK_FUNC, 0, true);

        import_function(kernel, evaluate_apply_feedback, "apply_feedback(Branch branch)->Branch "
            "'Experimental, creates terms to apply feedback in the given branch' end");
    }
}
}
