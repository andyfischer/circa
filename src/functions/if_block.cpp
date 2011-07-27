// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace if_block_function {

    void formatSource(StyledSource* source, Term* term)
    {
        Branch& contents = nested_contents(term);

        for (int branch_index=0; branch_index < contents.length(); branch_index++) {
            Term* branch_term = contents[branch_index];

            if (is_hidden(branch_term))
                continue;

            append_phrase(source,
                    branch_term->stringPropOptional("syntax:preWhitespace", ""),
                    branch_term, token::WHITESPACE);

            if (branch_index == 0) {
                append_phrase(source, "if ", branch_term, phrase_type::KEYWORD);
                format_source_for_input(source, branch_term, 0);
            } else if (branch_index < (contents.length()-2)) {
                append_phrase(source, "elif ", branch_term, phrase_type::KEYWORD);
                format_source_for_input(source, branch_term, 0);
            }
            else
                append_phrase(source, "else", branch_term, phrase_type::UNDEFINED);

            format_branch_source(source, nested_contents(branch_term), branch_term);
        }
    }

    int getOutputCount(Term* term)
    {
        Branch& contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents.length() == 0)
            return 1;

        Branch& outerRebinds = contents.getFromEnd(0)->contents();
        return outerRebinds.length() + 1;
    }

    const char* getOutputName(Term* term, int outputIndex)
    {
        Branch& contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents.length() == 0)
            return "";

        Branch& outerRebinds = contents.getFromEnd(0)->contents();
        return outerRebinds[outputIndex - 1]->name.c_str();
    }

    Type* getOutputType(Term* term, int outputIndex)
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

    Type* joinFunc_specializeType(Term* term)
    {
        if (term->input(0) == NULL || term->input(1) == NULL)
            return &ANY_T;
        List types;
        set_type_list(&types, get_type_of_input(term, 0), get_type_of_input(term, 1));
        return find_common_type(&types);
    }

    void setup(Branch& kernel)
    {
        IF_BLOCK_FUNC = import_function(kernel, evaluate_if_block, "if_block() -> any");
        get_function_attrs(IF_BLOCK_FUNC)->formatSource = formatSource;
        get_function_attrs(IF_BLOCK_FUNC)->getOutputCount = getOutputCount;
        get_function_attrs(IF_BLOCK_FUNC)->getOutputName = getOutputName;
        get_function_attrs(IF_BLOCK_FUNC)->getOutputType = getOutputType;

        JOIN_FUNC = import_function(kernel, NULL, "join(any...) -> any");
        get_function_attrs(JOIN_FUNC)->specializeType = joinFunc_specializeType;
    }
}
}
