// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace extra_output_function {

    Type* specializeType(Term* term)
    {
        ca_assert(term->input(0)->owningBranch == term->owningBranch);
        int myOutputIndex = term->index - term->input(0)->index;
        return get_output_type(term->input(0), myOutputIndex);
    }

    void setup(Branch* kernel)
    {
        EXTRA_OUTPUT_FUNC = import_function(kernel, NULL, "extra_output(any) -> any");
        as_function(EXTRA_OUTPUT_FUNC)->specializeType = specializeType;
    }
}
}
