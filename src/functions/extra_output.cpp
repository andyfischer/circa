// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace extra_output_function {

    Type* specializeType(Term* term)
    {
        ca_assert(term->input(0)->owningBlock == term->owningBlock);
        int myOutputIndex = term->index - term->input(0)->index;
        return get_output_type(term->input(0), myOutputIndex);
    }

    void setup(Block* kernel)
    {
        FUNCS.extra_output = import_function(kernel, NULL, "extra_output(any _) -> any");
        as_function(FUNCS.extra_output)->specializeType = specializeType;
        function_set_empty_evaluation(as_function(FUNCS.extra_output));
    }
}
}
