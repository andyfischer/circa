// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace comment_function {

    void setup(Block* kernel)
    {
        FUNCS.comment = import_function(kernel, NULL, "comment()");
        block_set_evaluation_empty(function_contents(FUNCS.comment), true);
    }
}
}
