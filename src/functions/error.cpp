// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace error_function {

    CA_FUNCTION(error)
    {
        RAISE_ERROR(STRING_INPUT(0));
    }

    void setup(Block* kernel)
    {
        FUNCS.error = import_function(kernel, error, "error(String msg)");
    }
}
}
