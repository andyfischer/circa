// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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
        function_t::set_input_meta(FEEDBACK_FUNC, 0, true);
    }
}
}
