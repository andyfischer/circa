// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace error_function {

    CA_FUNCTION(error)
    {
        RAISE_ERROR(STRING_INPUT(0));
    }

    CA_FUNCTION(errored)
    {
        set_bool(OUTPUT, is_error(INPUT(0)));
    }
    void setup(Branch* kernel)
    {
        FUNCS.error = import_function(kernel, error, "error(string)");
        ERRORED_FUNC = import_function(kernel, errored, "errored(any) -> bool");
    }
}
}
