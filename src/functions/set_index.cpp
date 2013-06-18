// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace set_index_function {

    void evaluate(caStack* stack)
    {
        INCREMENT_STAT(SetIndex);

        caValue* output = circa_output(stack, 0);
        copy(circa_input(stack, 0), output);
        touch(output);
        int index = circa_int_input(stack, 1);
        copy(circa_input(stack, 2), list_get(output, index));
    }

    Type* specializeType(Term* caller)
    {
        // TODO: Fix type inference on set_index.
        return TYPES.list;
        //return caller->input(0)->type;
    }

    void setup(Block* kernel)
    {
        FUNCS.set_index = import_function(kernel, evaluate,
                "set_index(any list, int index, any val) -> List");
        block_set_specialize_type_func(function_contents(FUNCS.set_index), specializeType);
    }
}
}
