// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace print_function {

    void evaluate(Term* caller)
    {
        for (int i = 0; i < caller->numInputs(); i++) {
            if (caller->input(i)->type == STRING_TYPE)
                std::cout << as_string(caller->input(i));
            else
                std::cout << caller->input(i)->toString();
        }
        std::cout << std::endl;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "print(any...)");
    }
}
} // namespace circa
