// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace make_function {

    void make_func(caStack* stack)
    {
        make(as_type(circa_input(stack, 0)), circa_output(stack, 0));
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
        Term* func = import_function(kernel, make_func, "make(Type t) -> any");
        block_set_specialize_type_func(as_function2(func), specializeType);
    }
}
}
