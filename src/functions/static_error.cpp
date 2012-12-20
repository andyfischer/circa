// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace static_error_function {

    void setup(Block* kernel)
    {
        FUNCS.static_error = 
            import_function(kernel, NULL, "static_error(any msg)");

        block_set_evaluation_empty(function_contents(FUNCS.static_error), true);
    }

}
}
