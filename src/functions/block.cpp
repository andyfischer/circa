// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace block_function {

    void format_source(caValue* source, Term* term)
    {
        format_name_binding(source, term);
        format_block_source(source, nested_contents(term), term);
    }

    void setup(Block* kernel)
    {
        // TODO: delete
        FUNCS.block_unevaluated = import_function(kernel, NULL, "block_unevaluated()");
        block_set_evaluation_empty(function_contents(FUNCS.block_unevaluated), true);
        block_set_function_has_nested(function_contents(FUNCS.block_unevaluated), true);

        // TODO: delete
        FUNCS.lambda = import_function(kernel, NULL, "lambda(any inputs :multiple) -> Block");
        block_set_format_source_func(function_contents(FUNCS.lambda), format_source);
        block_set_function_has_nested(function_contents(FUNCS.lambda), true);
    }
}
}
