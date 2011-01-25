// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "builtins.h"
#include "circa.h"

namespace circa {
namespace unknown_function_function {

    CA_FUNCTION(evaluate)
    {
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "unknown_function(any...) -> any");

        UNKNOWN_FUNCTION = main_func;
    }
}
}
