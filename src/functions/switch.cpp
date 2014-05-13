// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace switch_function {

    void evaluate_case(caStack* stack) { }
    void evaluate_default_case(caStack* stack) { }

    void setup(Block* kernel)
    {
        FUNCS.switch_func = import_function(kernel, NULL, "switch(any input :optional) -> any");
    }

} // namespace switch_function
} // namespace circa
