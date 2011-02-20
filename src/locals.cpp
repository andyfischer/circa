// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "term.h"

namespace circa {

int get_output_count(Term* term)
{
    if (!FINISHED_BOOTSTRAP)
        return 1;

    // check if the function has overridden getOutputCount
    FunctionAttrs::GetOutputCount getOutputCount = NULL;

    if (term->function == NULL)
        return 1;

    FunctionAttrs* attrs = get_function_attrs(term->function);

    if (attrs == NULL)
        return 1;
    
    getOutputCount = attrs->getOutputCount;

    if (getOutputCount != NULL)
        return getOutputCount(term);

    // Default behavior, if FunctionAttrs was found.
    return attrs->outputCount;
}
    
void update_locals_index_for_new_term(Term* term)
{
    Branch* branch = term->owningBranch;

    int numLocals = get_output_count(term);

    if (numLocals > 0) {
        term->localsIndex = branch->localsCount;
        branch->localsCount += numLocals;
    } else {
        term->localsIndex = -1;
    }
}

void refresh_locals_indices(Branch& branch, int startingAt)
{
    int nextLocal = 0;
    if (startingAt > 0) {
        Term* prev = branch[startingAt - 1];
        nextLocal = prev->localsIndex + get_output_count(prev);
    }

    for (int i=startingAt; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL)
            continue;
        int outputCount = get_output_count(term);
        if (outputCount == 0)
            continue;
        term->localsIndex = nextLocal;
        nextLocal += outputCount;
    }
    branch.localsCount = nextLocal;
}

} // namespace circa
