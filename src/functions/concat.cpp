// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace concat_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(concat, "concat(any :multiple) -> String;"
            "'Concatenate each input (converting to a string if necessary).'")
    {
        caValue* args = circa_input(STACK, 0);
        std::stringstream out;
        for (int index=0; index < circa_count(args); index++) {
            caValue* v = circa_index(args, index);
            if (is_string(v))
                out << as_string(v);
            else
                out << to_string(v);
        }
        set_string(OUTPUT, out.str());
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
