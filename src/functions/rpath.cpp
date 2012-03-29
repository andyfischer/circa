// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "../file_utils.h"

namespace circa {
namespace rpath_function {

    CA_FUNCTION(rpath)
    {
        circ_get_path_relative_to_source((caTerm*) CALLER, INPUT(0), OUTPUT);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, rpath, "def rpath(String) -> String");
    }
}
}
