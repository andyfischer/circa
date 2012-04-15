// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "../types/set.h"

namespace circa {
namespace container_set_function {

    CA_FUNCTION(evaluate)
    {
        caValue* result = OUTPUT;
        set_list(result, 0);

        for (int index=0; index < NUM_INPUTS; index++)
            set_t::add(result, INPUT(index));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "set(any...) -> Set");
    }
}
}
