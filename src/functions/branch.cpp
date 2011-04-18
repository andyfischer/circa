// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace branch_function {

    CA_FUNCTION(branch_evaluate)
    {
        evaluate_branch_internal_with_state(CONTEXT, CALLER);
    }

    CA_FUNCTION(lambda_evaluate)
    {
        branch_ref_function::set_branch_ref(OUTPUT, &CALLER->nestedContents);
    }

    void format_source(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        format_branch_source(source, term->nestedContents, term);
    }

    void setup(Branch& kernel)
    {
        BRANCH_FUNC = import_function(kernel, branch_evaluate, "branch()");
        get_function_attrs(BRANCH_FUNC)->formatSource = format_source;
        LAMBDA_FUNC = import_function(kernel, lambda_evaluate, "lambda()");
        get_function_attrs(LAMBDA_FUNC)->formatSource = format_source;
    }
}
}
