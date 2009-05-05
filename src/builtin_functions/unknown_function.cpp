// Copyright 2008 Andrew Fischer

#include "builtins.h"
#include "circa.h"

namespace circa {
namespace unknown_function_function {

    void evaluate(Term* caller)
    {
        std::cout << "Warning, calling an unknown function: "
            << caller->stringProperty("message") << std::endl;
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "unknown(any...) -> any");
        as_function(main_func).pureFunction = false;
        as_function(main_func).hasSideEffects = true;

        UNKNOWN_FUNCTION = main_func;
    }
}
}
