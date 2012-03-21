// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace return_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(return_func, "return(any :multiple :optional)")
    {
        // Grab input values
        List inputs;
        consume_inputs_to_list(CONTEXT, &inputs);

        // Pop frames
        while (!is_subroutine(top_frame(CONTEXT)->branch->owningTerm))
            finish_frame(CONTEXT);

        Branch* branch = top_frame(CONTEXT)->branch;

        // Copy values to their output placeholders.
        for (int i=0; i < inputs.length(); i++) {
            Term* output = get_output_placeholder(branch, i);
            caValue* out = get_register(CONTEXT, output);
            if (output == NULL)
                set_null(out);
            else
                move(inputs[i], out);
        }

        // Move PC to end
        Frame* top = top_frame(CONTEXT);
        top->nextPc = top->branch->length();
    }

    void formatSource(StyledSource* source, Term* term)
    {
        if (term->boolPropOptional("syntax:returnStatement", false)) {
            append_phrase(source, "return", term, phrase_type::KEYWORD);
            append_phrase(source,
                    term->stringPropOptional("syntax:postKeywordWs", " "),
                    term, phrase_type::WHITESPACE);

            if (term->input(0) != NULL)
                format_source_for_input(source, term, 0, "", "");
        } else {
            format_term_source_default_formatting(source, term);
        }
    }

    void setup(Branch* kernel)
    {
        // this function can be initialized early
        if (kernel->get("return") != NULL)
            return;

        CA_SETUP_FUNCTIONS(kernel);
        RETURN_FUNC = kernel->get("return");
        as_function(RETURN_FUNC)->formatSource = formatSource;
    }
}
}
