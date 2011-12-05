// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace namespace_function {

    CA_FUNCTION(evaluate)
    {
        evaluate_branch_internal(CONTEXT, nested_contents(CALLER));
    }

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "namespace ", term, phrase_type::KEYWORD);
        append_phrase(source, term->name, term, phrase_type::TERM_NAME);
        format_branch_source(source, nested_contents(term), term);
        append_phrase(source, term->stringPropOptional("syntax:preEndWs", ""),
                term, token::WHITESPACE);
    }

    void early_setup(Branch* kernel)
    {
        NAMESPACE_FUNC = import_function(kernel, evaluate, "namespace()");
        as_function(NAMESPACE_FUNC)->formatSource = format_source;
    }
    void setup(Branch* kernel) {}
}
}
