// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace block_function {

    void setup(Block* kernel)
    {
        // TODO: delete
        FUNCS.block_unevaluated = import_function(kernel, NULL, "block_unevaluated()");
        block_set_evaluation_empty(function_contents(FUNCS.block_unevaluated), true);
        block_set_function_has_nested(function_contents(FUNCS.block_unevaluated), true);

        // TODO: delete
        FUNCS.lambda = import_function(kernel, NULL, "lambda(any inputs :multiple) -> Block");
        block_set_function_has_nested(function_contents(FUNCS.lambda), true);
    }
}
}
