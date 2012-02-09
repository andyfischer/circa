// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace create_function {

    CA_FUNCTION(create_func)
    {
        create(as_type(INPUT(0)), OUTPUT);
    }

    Type* specializeType(Term* caller)
    {
        Term* input = caller->input(0);
        if (input == NULL)
            return &ANY_T;

        if (is_value(input))
            return as_type(input);

        return &ANY_T;
    }

    void setup(Branch* kernel)
    {
        Term* func = import_function(kernel, create_func, "create(Type) -> any");
        as_function(func)->specializeType = specializeType;
    }
}
}
