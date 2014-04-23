// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace unknown_identifier_function {

    void evaluate(caStack* stack)
    {
        Value msg;
        set_string(&msg, "Unknown identifier: ");
        string_append(&msg, term_name(circa_caller_term(stack)));
        circa_output_error(stack, as_cstring(&msg));
    }

    void setup(Block* kernel)
    {
        FUNCS.unknown_identifier = import_function(kernel, evaluate, "unknown_identifier() -> any");
    }
}
}
