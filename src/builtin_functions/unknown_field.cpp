// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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
