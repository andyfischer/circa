// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace concat_function {

    void concat(caStack* stack)
    {
        caValue* args = circa_input(stack, 0);
        std::stringstream out;
        for (int index=0; index < list_length(args); index++) {
            caValue* v = circa_index(args, index);
            if (is_string(v))
                out << as_string(v);
            else
                out << to_string(v);
        }
        set_string(circa_output(stack, 0), out.str());
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, concat, "concat(any :multiple) -> String;"
            "'Concatenate each input (converting to a string if necessary).'");
    }
}
}
