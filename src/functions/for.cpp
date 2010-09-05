// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace for_function {

    CA_FUNCTION(empty_evaluate)
    {
    }

    void format_heading(StyledSource* source, Term* term)
    {
        append_phrase(source, "for ", term, phrase_type::KEYWORD);
        append_phrase(source, get_for_loop_iterator(term)->name.c_str(),
                term, phrase_type::UNDEFINED);
        append_phrase(source, " in ", term, phrase_type::KEYWORD);
        format_source_for_input(source, term, 1);
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_heading(source, term);
        append_phrase(source, term->stringPropOptional("syntax:postHeadingWs", "\n"),
            term, token::WHITESPACE);
        format_branch_source(source, term->nestedContents, term);
        append_phrase(source, term->stringPropOptional("syntax:whitespaceBeforeEnd", ""),
            term, token::WHITESPACE);
    }

    CA_FUNCTION(evaluate_discard)
    {
    }

    void discard_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "discard", term, phrase_type::KEYWORD);
    }

    void setup(Branch& kernel)
    {
        FOR_FUNC = import_function(kernel, empty_evaluate, "for(Indexable) -> List");
        function_t::get_attrs(FOR_FUNC).formatSource = formatSource;
        function_t::set_exposed_name_path(FOR_FUNC, "#rebinds_for_outer");

        DISCARD_FUNC = import_function(kernel, evaluate_discard, "discard(any)");
        function_t::get_attrs(DISCARD_FUNC).formatSource = discard_formatSource;
        hide_from_docs(DISCARD_FUNC);
    }
}
} // namespace circa
