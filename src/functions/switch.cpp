// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "switch_block.h"

namespace circa {
namespace switch_function {

    CA_FUNCTION(evaluate_case)
    {
    }

    void switch_formatSource(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, "switch ", term, phrase_type::KEYWORD);
        format_source_for_input(source, term, 0);
        format_branch_source(source, nested_contents(term), term);
    }

    int switch_getOutputCount(Term* term)
    {
        Branch& contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents.length() == 0)
            return 1;

        Branch& outerRebinds = contents.getFromEnd(0)->contents();
        return outerRebinds.length() + 1;
    }

    const char* switch_getOutputName(Term* term, int outputIndex)
    {
        Branch& contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents.length() == 0)
            return "";

        Branch& outerRebinds = contents.getFromEnd(0)->contents();
        return outerRebinds[outputIndex - 1]->name.c_str();
    }
    Type* switch_getOutputType(Term* term, int outputIndex)
    {
        if (outputIndex == 0)
            return &VOID_T;

        Branch& contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents.length() == 0)
            return &ANY_T;

        Branch& outerRebinds = contents.getFromEnd(0)->contents();
        return outerRebinds[outputIndex - 1]->type;
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
        get_function_attrs(SWITCH_FUNC)->getOutputCount = switch_getOutputCount;
        get_function_attrs(SWITCH_FUNC)->getOutputName = switch_getOutputName;
        get_function_attrs(SWITCH_FUNC)->getOutputType = switch_getOutputType;

        CASE_FUNC = import_function(kernel, evaluate_case, "case(any input)");
        get_function_attrs(CASE_FUNC)->formatSource = case_formatSource;
    }

} // namespace switch_function
} // namespace circa
