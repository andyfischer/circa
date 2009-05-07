// Copyright 2008 Andrew Fischer

#include "builtins.h"
#include "circa.h"

namespace circa {
namespace unknown_function_function {

    void evaluate(Term* caller)
    {
        as_function(caller).outputType = ANY_TYPE;
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "unknown_function() : Function");
        as_function(main_func).pureFunction = false;
        as_function(main_func).hasSideEffects = true;

        UNKNOWN_FUNCTION = main_func;
    }
}
}
