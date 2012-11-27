// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace feedback_function {

    void evaluate(caStack* stack)
    {
        handle_feedback_event(stack, (Term*) circa_caller_input_term(stack, 0),
                circa_input(stack, 1));
    }

    void setup(Block* kernel)
    {
        FUNCS.feedback = import_function(kernel, evaluate, "feedback(any target :meta, any)");
    }
}
}
