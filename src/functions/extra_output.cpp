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
        block_set_specialize_type_func(as_function2(FUNCS.extra_output), specializeType);
        block_set_evaluation_empty(function_contents(FUNCS.extra_output), true);
    }
}
}
