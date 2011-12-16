// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace if_block_function {

    void formatSource(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);

        Branch* contents = nested_contents(term);

        int index = 0;
        while (contents->get(index)->function == INPUT_PLACEHOLDER_FUNC)
            index++;

        bool firstCase = true;

        for (; index < contents->length(); index++) {
            Term* caseTerm = contents->get(index);

            if (caseTerm->function != CASE_FUNC)
                break;

            if (is_hidden(caseTerm))
                continue;

            append_phrase(source,
                    caseTerm->stringPropOptional("syntax:preWhitespace", ""),
                    caseTerm, token::WHITESPACE);

            if (firstCase) {
                append_phrase(source, "if ", caseTerm, phrase_type::KEYWORD);
                format_source_for_input(source, caseTerm, 0);
                firstCase = false;
            } else if (caseTerm->input(0) != NULL) {
                append_phrase(source, "elif ", caseTerm, phrase_type::KEYWORD);
                format_source_for_input(source, caseTerm, 0);
            }
            else
                append_phrase(source, "else", caseTerm, phrase_type::UNDEFINED);

            format_branch_source(source, nested_contents(caseTerm), caseTerm);
        }
    }

    void setup(Branch* kernel)
    {
        IF_BLOCK_FUNC = import_function(kernel, evaluate_if_block, "if_block() -> any");
        as_function(IF_BLOCK_FUNC)->formatSource = formatSource;
        as_function(IF_BLOCK_FUNC)->createsStackFrame = false;
        as_function(IF_BLOCK_FUNC)->vmInstruction = ControlFlowCall;

        CASE_FUNC = import_function(kernel, NULL, "case(bool :optional)");
        as_function(CASE_FUNC)->createsStackFrame = true;
    }
}
}
