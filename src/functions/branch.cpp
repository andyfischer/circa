// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace branch_function {

    CA_FUNCTION(branch_evaluate)
    {
        evaluate_branch_internal_with_state(CONTEXT, CALLER);
    }

    void format_source(StyledSource* source, Term* term)
    {
        if (term->boolPropOptional("syntax:literal-list", false)) {
            format_name_binding(source, term);
            append_phrase(source, "[", term, token::LBRACKET);
            Branch& contents = term->nestedContents;

            for (int i=0; i < contents.length(); i++) {
                Term* term = contents[i];
                format_term_source(source, term);
            }

            append_phrase(source, "]", term, token::RBRACKET);
        } else {
            format_name_binding(source, term);
            format_branch_source(source, term->nestedContents, term);
        }
    }

    void setup(Branch& kernel)
    {
        BRANCH_FUNC = import_function(kernel, branch_evaluate, "branch()");
        get_function_attrs(BRANCH_FUNC)->formatSource = format_source;
    }
}
}
