// Copyright 2009 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

const std::string TRAINING_BRANCH_NAME = "#training";

bool is_trainable(Term* term)
{
    return term->boolPropertyOptional("trainable", false)
        || term->boolPropertyOptional("derived-trainable", false);
}

void update_derived_trainable_properties(Branch& branch)
{
    for (CodeIterator it(&branch); !it.finished(); it.advance()) {
        // if any of our inputs are trainable then mark us as derived-trainable
        bool found = false;
        for (int i=0; i < it->numInputs(); i++) {
            if (is_trainable(it->input(i))) {
                found = true;
                break;
            }
        }

        it->boolProperty("derived-trainable") = found;
    }
}

void generate_training(Branch& branch, Term* subject, Term* desired)
{
    Function& targetsFunction = as_function(subject->function);

    if (targetsFunction.generateTraining != NULL)
    {
        targetsFunction.generateTraining(branch, subject, desired);
    } else {
        std::cout << "warn: function " << targetsFunction.name <<
            " doesn't have a generateTraining function" << std::endl;
        return;
    }
}

void refresh_training_branch(Branch& branch)
{
    update_derived_trainable_properties(branch);

    // Check if '#training' branch exists. Create if it doesn't exist
    if (!branch.contains(TRAINING_BRANCH_NAME)) {
        apply(&branch, BRANCH_FUNC, RefList(), TRAINING_BRANCH_NAME);
    }

    Branch& trainingBranch = as_branch(branch[TRAINING_BRANCH_NAME]);

    trainingBranch.clear();

    // Generate training for every feedback() function in this branch
    for (int i = 0; i < branch.numTerms(); i++) {
        Term* term = branch[i];
        if (term->function == FEEDBACK_FUNC) {
            generate_training(trainingBranch, term->input(0), term->input(1));
        }
    }

    // TODO: Normalize trainingBranch
}

} // namespace circa
