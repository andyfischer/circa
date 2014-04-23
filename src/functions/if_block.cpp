// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace if_block_function {

    void setup(Block* kernel)
    {
        FUNCS.if_block = import_function(kernel, NULL, "if() -> any");

        FUNCS.case_func = import_function(kernel, NULL, "case(bool b :optional)");

        block_set_function_has_nested(function_contents(FUNCS.if_block), true);
        block_set_function_has_nested(function_contents(FUNCS.case_func), true);
    }
}
}
