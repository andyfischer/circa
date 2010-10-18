// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace namespace_function {

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "namespace ", term, phrase_type::KEYWORD);
        append_phrase(source, term->name, term, phrase_type::TERM_NAME);
        append_phrase(source, term->stringPropOptional("syntax:postHeadingWs", "\n"),
                term, token::WHITESPACE);
        format_branch_source(source, term->nestedContents, NULL);
        append_phrase(source, term->stringPropOptional("syntax:preEndWs", ""),
                term, token::WHITESPACE);
                
        append_phrase(source, "end", term, phrase_type::KEYWORD);
    }

    void early_setup(Branch& kernel)
    {
        NAMESPACE_FUNC = import_function(kernel, branch_function::branch_evaluate, "namespace()");
        function_t::get_attrs(NAMESPACE_FUNC).formatSource = format_source;
    }
    void setup(Branch& kernel) {}
}
}
