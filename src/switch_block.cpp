// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "if_block.h"
#include "importing_macros.h"
#include "term.h"
#include "type.h"

namespace circa {

void switch_block_post_compile(Term* term)
{
    // Add a default case
    apply(nested_contents(term), FUNCS.default_case, TermList());
    finish_if_block(term);
}

CA_FUNCTION(evaluate_switch)
{
#if 0
    EvalContext* context = CONTEXT;
    Branch* contents = nested_contents(CALLER);
    caValue* input = INPUT(0);

    // Iterate through each 'case' and find one that succeeds.
    for (int caseIndex=0; caseIndex < contents->length()-1; caseIndex++) {
        Term* caseTerm = contents->get(caseIndex);

        bool succeeds = false;
        if (caseTerm->function == DEFAULT_FUNCS.case_func) {
            succeeds = true;
        } else if (caseTerm->function == FUNCS.case_func) {
            caValue* caseValue = get_input(context, caseTerm, 0);
            succeeds = equals(input, caseValue);
        } else {
            internal_error("unrecognized function inside switch()");
        }

        if (succeeds) {
            Branch* caseContents = nested_contents(caseTerm);
            push_frame(context, caseContents);

            for (int i=0; i < caseContents->length(); i++) {
                evaluate_single_term(context, caseContents->get(i));

                  if (error_occurred(context))
                      break;
            }

            // Copy joined values to output slots
            Branch* joining = nested_contents(contents->getFromEnd(0));

            for (int i=0; i < joining->length(); i++) {
                Term* joinTerm = joining->get(i);
                caValue* value = get_input(context, joinTerm, caseIndex);

                ca_test_assert(cast_possible(value, get_output_type(CALLER, i+1)));

                copy(value, EXTRA_OUTPUT(i));
            }

            pop_frame(context);
            break;
        }
    }
#endif
}

} // namespace circa
