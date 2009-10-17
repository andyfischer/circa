// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved

#include <circa.h>

namespace circa {

void recursively_evaluate_inputs_inside_branch(Term* term, Branch* branch)
{
    if (term->owningBranch != branch)
        return;

    evaluate_term(term);

    for (int i=0; i < term->numInputs(); i++)
        recursively_evaluate_inputs_inside_branch(term->input(i), branch);
}

void lift_closure(Branch& branch)
{
    // TODO: Should do an evaluate_with_no_side_effects here (when it exists)

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL) continue;
        if (term->function == FREEZE_FUNC) {
            Term* input = term->input(0);
            // This is flawed (see above comment)
            recursively_evaluate_inputs_inside_branch(input, branch[i]->owningBranch);

            change_function(term, VALUE_FUNC);
        }
    }
}

} // namespace circa
