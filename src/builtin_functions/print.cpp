// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace print_function {

    CA_FUNCTION(evaluate)
    {
        for (int i = 0; i < NUM_INPUTS; i++) {
            if (INPUT(i)->value_type == type_contents(STRING_TYPE))
                std::cout << as_string(INPUT(i));
            else
                std::cout << INPUT(i)->toString();
        }
        std::cout << std::endl;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "print(any...) "
                "'Prints a line of text output to the console' end");

        // Alias
        import_function(kernel, evaluate, "trace(any...) "
                "'Prints a line of text output to the console' end");
    }
}
} // namespace circa
