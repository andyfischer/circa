// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace print_function {

    CA_FUNCTION(evaluate)
    {
        caValue* args = circa_input(_stack, 0);

        for (int i = 0; i < circa_count(args); i++) {
            caValue* val = circa_index(args, i);
            if (is_string(val))
                std::cout << as_string(val);
            else
                std::cout << to_string(val);
        }
        std::cout << std::endl;

        set_null(circa_output(_stack, 0));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate, "print(any items :multiple) "
                "-- Prints a line of text output to the console.");

        import_function(kernel, evaluate, "trace(any items :multiple) "
                "-- Prints a line of text output to the console.");
    }
}
} // namespace circa
