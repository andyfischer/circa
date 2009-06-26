// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace feedback_function {

    void evaluate(Term* caller)
    {
        // No-op
    }

    void setup(Branch& kernel)
    {
        FEEDBACK_FUNC = import_function(kernel, evaluate, "feedback(any,any)");
        function_set_input_meta(FEEDBACK_FUNC, 0, true);
    }
}
}
