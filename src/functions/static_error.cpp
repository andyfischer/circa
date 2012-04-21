// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace static_error_function {

    void setup(Branch* kernel)
    {
        STATIC_ERROR_FUNC = 
            import_function(kernel, NULL, "static_error(any msg)");

        function_set_empty_evaluation(as_function(STATIC_ERROR_FUNC));
    }

}
}
