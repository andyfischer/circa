// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "switch_block.h"

namespace circa {
namespace switch_function {

    CA_FUNCTION(evaluate_case)
    {
        // TODO
    }

    void switch_formatSource(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, "switch ", term, phrase_type::KEYWORD);
        format_source_for_input(source, term, 0);
        format_branch_source(source, nested_contents(term), term);
    }

    void case_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "case ", term, phrase_type::KEYWORD);
        format_source_for_input(source, term, 0);
        format_branch_source(source, nested_contents(term), term);
    }

    void setup(Branch& kernel)
    {

        SWITCH_FUNC = import_function(kernel, evaluate_switch, "switch(any input) -> any");
        get_function_attrs(SWITCH_FUNC)->formatSource = switch_formatSource;

        CASE_FUNC = import_function(kernel, evaluate_case, "case(any input)");
        get_function_attrs(CASE_FUNC)->formatSource = case_formatSource;
    }

} // namespace switch_function
} // namespace circa
