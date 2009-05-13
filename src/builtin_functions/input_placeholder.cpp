// Copyright 2008 Paul Hodge

#include <circa.h>

// This function is used inside subroutines, as a placeholder for an incoming
// input value.

namespace circa {
namespace input_placeholder_function {

    void evaluate(Term* caller)
    {
    }

    /*
    void generateFeedback(Branch& branch, Term* subject, Term* desired)
    {
        // Just lay down a feedback() term here, we'll find it in subroutine_feedback
        apply(&branch, FEEDBACK_FUNC, RefList(subject, desired));
    }
    */

    void setup(Branch& kernel)
    {
        INPUT_PLACEHOLDER_FUNC = import_function(kernel, evaluate, "input_placeholder() : any");
    }
}
}
