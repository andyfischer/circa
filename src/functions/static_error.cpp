// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace static_error_function {

    void setup(Branch* kernel)
    {
        FUNCS.static_error = 
            import_function(kernel, NULL, "static_error(any msg)");

        function_set_empty_evaluation(as_function(FUNCS.static_error));
    }

}
}
