// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace branch_function {

    CA_FUNCTION(branch_evaluate)
    {
        evaluate_branch_internal_with_state(CONTEXT, CALLER, nested_contents(CALLER));
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

    void setup(Branch& kernel)
    {
        BRANCH_FUNC = import_function(kernel, branch_evaluate, "branch()");
        get_function_attrs(BRANCH_FUNC)->formatSource = format_source;
        BRANCH_UNEVALUATED_FUNC = import_function(kernel, NULL, "branch_unevaluated()");
        LAMBDA_FUNC = import_function(kernel, lambda_evaluate, "lambda() -> Branch");
        get_function_attrs(LAMBDA_FUNC)->formatSource = format_source;
    }
}
}
