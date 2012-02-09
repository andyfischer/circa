// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace branch_function {

    CA_FUNCTION(branch_evaluate)
    {
        push_frame(CONTEXT, nested_contents(CALLER));
    }

    CA_FUNCTION(lambda_evaluate)
    {
        set_branch(OUTPUT, CALLER->nestedContents);
    }

    void format_source(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        format_branch_source(source, nested_contents(term), term);
    }

    void setup(Branch* kernel)
    {
        FUNCS.branch = import_function(kernel, branch_evaluate, "branch()");
        as_function(FUNCS.branch)->formatSource = format_source;

        BRANCH_UNEVALUATED_FUNC = import_function(kernel, NULL, "branch_unevaluated()");
        LAMBDA_FUNC = import_function(kernel, lambda_evaluate, "lambda() -> Branch");
        as_function(LAMBDA_FUNC)->formatSource = format_source;
    }
}
}
