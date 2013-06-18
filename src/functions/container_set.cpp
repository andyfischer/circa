// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "../types/set.h"

namespace circa {
namespace container_set_function {

    void evaluate(caStack* stack)
    {
        caValue* result = circa_output(stack, 0);
        set_list(result, 0);

        caValue* args = circa_input(stack, 0);

        for (int index=0; index < circa_count(args); index++)
            set_t::add(result, circa_index(args, index));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate, "set(any items :multiple) -> Set");
    }
}
}
