// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace branch_function {

    void format_source(caValue* source, Term* term)
    {
        format_name_binding(source, term);
        format_branch_source(source, nested_contents(term), term);
    }

    void setup(Branch* kernel)
    {
        FUNCS.branch = import_function(kernel, NULL, "branch()");
        as_function(FUNCS.branch)->formatSource = format_source;

        FUNCS.branch_unevaluated = import_function(kernel, NULL, "branch_unevaluated()");
        function_set_empty_evaluation(as_function(FUNCS.branch_unevaluated));

        FUNCS.lambda = import_function(kernel, NULL, "lambda(any :multiple) -> Branch");
        as_function(FUNCS.lambda)->formatSource = format_source;
    }
}
}
