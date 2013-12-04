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
                    caseTerm->stringProp(sym_Syntax_PreWs, ""),
                    caseTerm, tok_Whitespace);

            if (firstCase) {
                append_phrase(source, "if ", caseTerm, sym_Keyword);
                Term* conditionCheck = case_find_condition_check(nested_contents(caseTerm));
                if (conditionCheck != NULL)
                    format_source_for_input(source, conditionCheck, 0);
                firstCase = false;
            } else if (caseTerm->name != "else") {
                append_phrase(source, "elif ", caseTerm, sym_Keyword);
                Term* conditionCheck = case_find_condition_check(nested_contents(caseTerm));
                if (conditionCheck != NULL)
                    format_source_for_input(source, conditionCheck, 0);
            }
            else
                append_phrase(source, "else", caseTerm, sym_None);

            // whitespace following the if/elif/else
            append_phrase(source,
                    caseTerm->stringProp(sym_Syntax_LineEnding, ""),
                    caseTerm, tok_Whitespace);

            format_block_source(source, nested_contents(caseTerm), caseTerm);
        }
    }

    void setup(Block* kernel)
    {
        FUNCS.if_block = import_function(kernel, NULL, "if() -> any");
        block_set_format_source_func(function_contents(FUNCS.if_block), formatSource);

        FUNCS.case_func = import_function(kernel, NULL, "case(bool b :optional)");

        block_set_function_has_nested(function_contents(FUNCS.if_block), true);
        block_set_function_has_nested(function_contents(FUNCS.case_func), true);
    }
}
}
