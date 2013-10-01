// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace error_function {

    void error(caStack* stack)
    {
        circa_output_error(stack, circa_string_input(stack, 0));
    }

    void setup(Block* kernel)
    {
        FUNCS.error = import_function(kernel, error, "error(String msg)");
    }
} // namespace error_function
} // namespace circa
