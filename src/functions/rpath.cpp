// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "file.h"

namespace circa {
namespace rpath_function {

    void rpath(caStack* stack)
    {
        caBlock* block = circa_caller_block(stack);
        get_path_relative_to_source(block, circa_input(stack, 0), circa_output(stack, 0));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, rpath, "def rpath(String) -> String");
    }
}
}
