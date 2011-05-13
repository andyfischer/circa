// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace feedback_function {

    CA_FUNCTION(evaluate)
    {
        handle_feedback_event(CONTEXT, INPUT_TERM(0), INPUT(1));
    }

    void setup(Branch& kernel)
    {
        FEEDBACK_FUNC = import_function(kernel, evaluate, "feedback(any,any)");
        function_t::set_input_meta(FEEDBACK_FUNC, 0, true);
    }
}
}
