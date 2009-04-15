// Copyright 2008 Paul Hodge

#include "builtins.h"
#include "circa.h"

namespace circa {
namespace unknown_function_function {

    void evaluate(Term* caller)
    {
        std::cout << "Warning, calling an unknown function: "
            << as_string(caller->state) << std::endl;
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "unknown(any) -> any");
        as_function(main_func).stateType = STRING_TYPE;
        as_function(main_func).pureFunction = false;
        as_function(main_func).hasSideEffects = true;
        as_function(main_func).variableArgs = true;

        UNKNOWN_FUNCTION = main_func;
    }
}
}
