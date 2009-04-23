// Copyright 2008 Andrew Fischer

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
        Term* main_func = import_function(kernel, evaluate, "print(any...)");
        as_function(main_func).pureFunction = false;
    }
}
} // namespace circa
