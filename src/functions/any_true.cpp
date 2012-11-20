// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace any_true_function {

    void any_true(caStack* stack)
    {
        caValue* input = circa_input(stack, 0);

        int numElements = list_length(input);

        bool result = false;
        for (int i=0; i < numElements; i++)
            if (as_bool(list_get(input,i))) {
                result = true;
                break;
            }

        set_bool(circa_output(stack, 0), result);
    }

    void setup(Block* kernel)
    {
        import_function(kernel, any_true, "any_true(List l) -> bool;"
                "'Return whether any of the items in l are true'");
    }
}
} // namespace circa
