// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "../types/set.h"

namespace circa {
namespace set_union_function {

    CA_FUNCTION(evaluate)
    {
        caValue* result = OUTPUT;
        set_list(result, 0);

        for (int inputIndex=0; inputIndex < NUM_INPUTS; inputIndex++) {
            caValue* input = INPUT(inputIndex);
            int numElements = circa_count(input);
            for (int i=0; i < numElements; i++)
                set_t::add(result, circa_index(input,i));
        }
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "set_union(Set...) -> Set");
    }
}
}
