// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace concat_function {

    void concat(caStack* stack)
    {
        caValue* args = circa_input(stack, 0);
        caValue* out = circa_output(stack, 0);
        set_string(out, "");

        for (int index=0; index < list_length(args); index++) {
            caValue* v = circa_index(args, index);
            string_append(out, v);
        }
    }

    void setup(Block* kernel)
    {
        import_function(kernel, concat, "concat(any vals :multiple) -> String;"
            "'Concatenate each input (converting to a string if necessary).'");
    }
}
}
