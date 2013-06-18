// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace print_function {

    void evaluate(caStack* stack)
    {
        caValue* args = circa_input(stack, 0);

        std::stringstream out;

        for (int i = 0; i < circa_count(args); i++) {
            caValue* val = circa_index(args, i);
            if (is_string(val))
                out << as_string(val);
            else
                out << to_string(val);
        }

        write_log(out.str().c_str());

        set_null(circa_output(stack, 0));
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
