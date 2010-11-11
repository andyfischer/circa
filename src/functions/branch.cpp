// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace branch_function {

    CA_FUNCTION(branch_evaluate)
    {
        Branch& contents = CALLER->nestedContents;
        evaluate_branch_in_new_frame(CONTEXT, contents, NULL);
    }

    CA_FUNCTION(branch_evaluate_preserve_stack)
    {
        Branch& contents = CALLER->nestedContents;
        evaluate_branch(CONTEXT, contents);
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
        function_t::get_attrs(BRANCH_FUNC).formatSource = format_source;

        Term* branch_preserve_stack = import_function(kernel, branch_evaluate_preserve_stack,
                "branch_preserve_stack()");
        function_t::get_attrs(branch_preserve_stack).formatSource = format_source;
    }
}
}
