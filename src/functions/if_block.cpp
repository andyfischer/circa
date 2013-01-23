// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace if_block_function {

    void formatSource(caValue* source, Term* term)
    {
        format_name_binding(source, term);

        Block* contents = nested_contents(term);

        int index = 0;
        while (contents->get(index)->function == FUNCS.input)
            index++;

        bool firstCase = true;

        for (; index < contents->length(); index++) {
            Term* caseTerm = contents->get(index);

            if (caseTerm->function != FUNCS.case_func)
                break;

            if (is_hidden(caseTerm))
                continue;

            append_phrase(source,
                    caseTerm->stringProp("syntax:preWhitespace", ""),
                    caseTerm, tok_Whitespace);

            if (firstCase) {
                append_phrase(source, "if ", caseTerm, sym_Keyword);
                format_source_for_input(source, caseTerm, 0);
                firstCase = false;
            } else if (caseTerm->input(0) != NULL) {
                append_phrase(source, "elif ", caseTerm, sym_Keyword);
                format_source_for_input(source, caseTerm, 0);
            }
            else
                append_phrase(source, "else", caseTerm, sym_None);

            // whitespace following the if/elif/else
            append_phrase(source,
                    caseTerm->stringProp("syntax:lineEnding", ""),
                    caseTerm, tok_Whitespace);

            format_block_source(source, nested_contents(caseTerm), caseTerm);
        }
    }

    void setup(Block* kernel)
    {
        FUNCS.if_block = import_function(kernel, NULL, "if_block() -> any");
        as_function(FUNCS.if_block)->formatSource = formatSource;

        FUNCS.case_func = import_function(kernel, NULL, "case(bool b :optional)");
    }
}
}
