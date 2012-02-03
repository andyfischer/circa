// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa_internal.h"
#include "types/set.h"

namespace circa {
namespace container_set_function {

    CA_FUNCTION(evaluate)
    {
        List* result = List::checkCast(OUTPUT);
        result->clear();

        for (int index=0; index < NUM_INPUTS; index++)
            set_t::add(result, INPUT(index));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "set(any...) -> Set");
    }
}
}
