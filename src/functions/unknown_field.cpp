// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace unknown_field_function {

    CA_FUNCTION(evaluate)
    {
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "unknown_field(any...) -> any");

        UNKNOWN_FIELD_FUNC = main_func;
    }
}
}
