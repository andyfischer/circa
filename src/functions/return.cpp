// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace return_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(return_func, "return(any +optional)")
    {
        CONTEXT->interruptSubroutine = true;

        Branch& contents = CALLER->nestedContents;
        for (int i=0; i < contents.length(); i++)
            evaluate_single_term(CONTEXT, contents[i]);
    }

    void returnPostCompile(Term* returnCall)
    {
        Branch& contents = returnCall->nestedContents;
        contents.clear();

        // Iterate through every open state var in the subroutine that occurs before
        // the return(). If any were found, append a call to preserve_state_result().

        Term* sub = find_enclosing_subroutine(returnCall);

        if (sub == NULL)
            return;

        UpwardIterator it(returnCall);
        it.stopAt(sub->owningBranch);

        for ( ; it.unfinished(); it.advance()) {
            //if (format_global_id(*it) == "$1982") asm { int 3};
            Term* previousTerm = *it;
            if (previousTerm == NULL)
                continue;

            if (previousTerm == returnCall)
                break;

            if (previousTerm->function == GET_STATE_FIELD_FUNC) {
                if (previousTerm->name == "")
                    continue;
                Term* outcome = get_named_at(returnCall, previousTerm->name);
                apply(contents, PRESERVE_STATE_RESULT_FUNC, RefList(outcome));
            }
        }

        // Look for the enclosing subroutine, if found then add a call to
        // subroutine_output()
        {
            RefList inputs(returnCall->input(0));

            // Check for extra outputs, if found then include their results in this output
            int numInputs = function_t::num_inputs(sub);
            for (int i=0; i < numInputs; i++) {
                if (function_can_rebind_input(sub, i)) {
                    std::string const& name =
                        function_t::get_input_placeholder(sub, i)->name;
                    Term* result = get_named_at(returnCall, name);
                    inputs.append(result);
                }
            }

            if (SUBROUTINE_OUTPUT_FUNC != NULL)
                apply(contents, SUBROUTINE_OUTPUT_FUNC, inputs);
        }

        //std::cout << "finished returnPostCompile for " << format_global_id(returnCall) << std::endl;
        //dump(returnCall->nestedContents);
    }

    void setup(Branch& kernel)
    {
        // this function can be initialized early
        if (kernel["return"] != NULL)
            return;

        CA_SETUP_FUNCTIONS(kernel);
        RETURN_FUNC = kernel["return"];
        get_function_attrs(RETURN_FUNC)->postCompile = returnPostCompile;
    }
}
}
