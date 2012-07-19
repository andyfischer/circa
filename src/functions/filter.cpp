// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace filter_function {

    CA_FUNCTION(evaluate)
    {
        caValue* inputs = INPUT(0);
        caValue* bools = INPUT(1);

        int numInputs = num_elements(inputs);
        int numBools = num_elements(bools);

        if (numInputs != numBools)
            return RAISE_ERROR("Lists have different lengths");

        // Run through once to count # of trues
        int count = 0;
        for (int i=0; i < numInputs; i++)
            if (as_bool(get_index(bools,i)))
                count++;
        
        caValue* output = set_list(OUTPUT, count);

        int write = 0;
        for (int i=0; i < numInputs; i++) {
            if (as_bool(get_index(bools,i))) {
                copy(list_get(inputs,i), get_index(output, write++));
            }
        }
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "filter(Indexable,Indexable) -> List");
    }
}
} // namespace circa
