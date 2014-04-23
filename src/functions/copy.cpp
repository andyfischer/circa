// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace copy_function {

    void evaluate(caStack* stack)
    {
        copy(circa_input(stack, 0), circa_output(stack, 0));
    }

    Type* specializeType(Term* caller)
    {
        return get_type_of_input(caller, 0);
    }

    void setup(Block* kernel)
    {
        FUNCS.copy = import_function(kernel, evaluate, "copy(any val) -> any");
        block_set_specialize_type_func(function_contents(FUNCS.copy), specializeType);
    }
}
}
