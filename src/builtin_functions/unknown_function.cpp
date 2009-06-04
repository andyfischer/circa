// Copyright 2008 Paul Hodge

#include "builtins.h"
#include "circa.h"

namespace circa {
namespace unknown_function_function {

    void evaluate(Term* caller)
    {
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "unknown_function(any...) : any");
        as_function(main_func).pureFunction = false;
        as_function(main_func).hasSideEffects = true;

        UNKNOWN_FUNCTION = main_func;
    }
}
}
