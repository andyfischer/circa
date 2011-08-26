// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "importing_macros.h"
#include "term.h"

namespace circa {

CA_FUNCTION(evaluate_switch)
{
    EvalContext* context = CONTEXT;
    Branch& contents = nested_contents(CALLER);
    TaggedValue* input = INPUT(0);

    // Iterate through each 'case' and find one that equals the input
    for (int i=0; i < contents.length(); i++) {
        Term* caseTerm = contents[i];
        ca_assert(caseTerm->function == CASE_FUNC);
        TaggedValue* caseValue = get_input(context, caseTerm, 0);
        if (equals(input, caseValue)) {
            evaluate_branch_internal(context, nested_contents(caseTerm));
            break;
        }
    }
}

} // namespace circa
