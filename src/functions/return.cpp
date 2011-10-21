// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace return_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(return_func, "return(any :optional)")
    {
        CONTEXT->interruptSubroutine = true;

        Branch* contents = nested_contents(CALLER);
        start_using(contents);
        for (int i=0; i < contents->length(); i++)
            evaluate_single_term(CONTEXT, contents->get(i));
        finish_using(contents);
    }

    void returnPostCompile(Term* returnCall)
    {
        Branch* contents = nested_contents(returnCall);
        contents->clear();

        // Iterate through every open state var in the subroutine that occurs before
        // the return(). If any were found, append a call to preserve_state_result().

        Term* sub = find_enclosing_subroutine(returnCall);

        if (sub == NULL)
            return;

        UpwardIterator it(returnCall);
        it.stopAt(sub->owningBranch);

        for ( ; it.unfinished(); it.advance()) {
            Term* previousTerm = *it;
            if (previousTerm == NULL)
                continue;

            if (previousTerm == returnCall)
                break;

            if (previousTerm->function == GET_STATE_FIELD_FUNC) {
                if (previousTerm->name == "")
                    continue;
                Term* outcome = get_named_at(returnCall, previousTerm->name);
                apply(contents, PRESERVE_STATE_RESULT_FUNC, TermList(outcome));
            }
        }

        // Look for the enclosing subroutine, if found then add a call to
        // subroutine_output()
        {
            TermList inputs(returnCall->input(0));

            // Check for extra outputs, if found then include their results in this output

            FunctionAttrs* subAttrs = get_function_attrs(sub);

            int numInputs = function_num_inputs(subAttrs);
            for (int i=0; i < numInputs; i++) {
                if (function_can_rebind_input(sub, i)) {
                    std::string const& name =
                        function_get_input_placeholder(subAttrs, i)->name;
                    Term* result = get_named_at(returnCall, name);
                    inputs.append(result);
                }
            }

            if (SUBROUTINE_OUTPUT_FUNC != NULL)
                apply(contents, SUBROUTINE_OUTPUT_FUNC, inputs);
        }
    }

    void formatSource(StyledSource* source, Term* term)
    {
        if (term->boolPropOptional("syntax:returnStatement", false)) {
            append_phrase(source, "return", term, phrase_type::KEYWORD);
            append_phrase(source,
                    term->stringPropOptional("syntax:postKeywordWs", " "),
                    term, phrase_type::WHITESPACE);

            if (term->input(0) != NULL)
                format_source_for_input(source, term, 0);
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
        get_function_attrs(RETURN_FUNC)->postCompile = returnPostCompile;
        get_function_attrs(RETURN_FUNC)->formatSource = formatSource;
    }
}
}
