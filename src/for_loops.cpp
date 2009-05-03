// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {

Term* get_for_loop_iterator(Term* forTerm)
{
    return get_for_loop_code(forTerm)[0];
}

Branch& get_for_loop_code(Term* forTerm)
{
    return forTerm->state->field("code")->asBranch();
}

void evaluate_for_loop(Term* forTerm, Term* listTerm)
{
    Branch& codeBranch = get_for_loop_code(forTerm);
    Branch& stateBranch = forTerm->state->field("_state")->asBranch();

    int numIterations = as_branch(listTerm).numTerms();

    resize_list(stateBranch, as_branch(listTerm).numTerms(), BRANCH_TYPE);

    for (int i=0; i < numIterations; i++) {
        // Set up state: duplicate code once for each iteration. This should
        // be optimized.

        Branch& iterationBranch = stateBranch[i]->asBranch();
        
        duplicate_branch(codeBranch, iterationBranch);

        // Inject iterator value
        Term* iterator = iterationBranch[0];
        if (!value_fits_type(listTerm->field(i), iterator->type)) {
            error_occured(forTerm, "Internal error in evaluate_for_loop: can't assign this element to iterator");
            return;
        }
        assign_value(listTerm->field(i), iterator);

        // Evaluate
        evaluate_branch(iterationBranch);
    }
}

} // namespace circa
