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
        Term* main_func = import_c_function(kernel, evaluate,
                "function unknown(any) -> any");
        as_function(main_func).stateType = STRING_TYPE;

        UNKNOWN_FUNCTION = main_func;
    }
}
}
