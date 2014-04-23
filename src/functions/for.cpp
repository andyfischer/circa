// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace for_function {

    void setup(Block* kernel)
    {
        FUNCS.for_func = import_function(kernel, NULL, "for(List list) -> List");

        FUNCS.loop_iterator = import_function(kernel, NULL,
            "loop_iterator(any _, any _) -> int");
        FUNCS.loop_index = import_function(kernel, NULL, "loop_index(int index) -> int");
        block_set_evaluation_empty(function_contents(FUNCS.loop_index), true);

        FUNCS.loop_output_index = import_function(kernel, NULL, "loop_output_index() -> any");
        block_set_evaluation_empty(function_contents(FUNCS.loop_output_index), true);

        block_set_function_has_nested(function_contents(FUNCS.for_func), true);
    }
}
} // namespace circa
