// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace make_function {

    CA_FUNCTION(make_func)
    {
        make(as_type(INPUT(0)), OUTPUT);
    }

    Type* specializeType(Term* caller)
    {
        Term* input = caller->input(0);
        if (input == NULL)
            return TYPES.any;

        if (is_value(input) && is_type(input))
            return as_type(input);

        return TYPES.any;
    }

    void setup(Block* kernel)
    {
        Term* func = import_function(kernel, make_func, "make(Type) -> any");
        as_function(func)->specializeType = specializeType;
    }
}
}
