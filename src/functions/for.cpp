// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace for_function {

#ifdef BYTECODE
    CA_FUNCTION(empty_evaluate)
    {
    }
#endif

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
#ifndef BYTECODE
        Term* forTerm = INPUT_TERM(0);
        Term* discardCalled = get_for_loop_discard_called(forTerm);
        set_bool(discardCalled, true);
#endif
    }

    void discard_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "discard", term, phrase_type::KEYWORD);
    }

    void setup(Branch& kernel)
    {
#ifdef BYTECODE
        FOR_FUNC = import_function(kernel, empty_evaluate, "for(Indexable)");
#else
        FOR_FUNC = import_function(kernel, evaluate_for_loop,
                "for(List _state, Indexable) -> Branch");
        function_t::set_input_meta(FOR_FUNC, 0, true); // allow _state to be NULL
#endif
        function_t::get_attrs(FOR_FUNC).formatSource = formatSource;
        function_t::set_exposed_name_path(FOR_FUNC, "#rebinds_for_outer");

        DISCARD_FUNC = import_function(kernel, evaluate_discard, "discard(any)");
        function_t::get_attrs(DISCARD_FUNC).formatSource = discard_formatSource;
        hide_from_docs(DISCARD_FUNC);
    }
}
} // namespace circa
