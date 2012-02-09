// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace print_function {

    CA_FUNCTION(evaluate)
    {
        for (int i = 0; i < NUM_INPUTS; i++) {
            if (is_string(INPUT(i)))
                std::cout << as_string(INPUT(i));
            else
                std::cout << INPUT(i)->toString();
        }
        std::cout << std::endl;
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "print(any...) "
                "'Prints a line of text output to the console'");

        import_function(kernel, evaluate, "trace(any...) "
                "'Prints a line of text output to the console'");
    }
}
} // namespace circa
