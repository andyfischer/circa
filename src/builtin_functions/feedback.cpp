// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace feedback_function {

    void evaluate(Term* caller)
    {
        // No-op
    }

    void setup(Branch& kernel)
    {
        FEEDBACK_FUNC = import_function(kernel, evaluate, "function feedback(any,any)");
        as_function(FEEDBACK_FUNC).setInputMeta(0, true);
    }
}
}
