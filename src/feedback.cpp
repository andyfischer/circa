// Copyright 2009 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {

const std::string TRAINING_BRANCH_NAME = "#training";

bool is_trainable(Term* term)
{
    return term->boolPropOptional("trainable", false)
        || term->boolPropOptional("derived-trainable", false);
}

void set_trainable(Term* term, bool value)
{
    term->boolProp("trainable") = value;
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

        it->boolProp("derived-trainable") = found;
    }
}

void normalize_training_branch(Branch& branch)
{
    // Look for any terms that have multiple assign() functions, and combine them with
    // a feedback-accumulator to one assign()
    
    // First, make a map of every assigned-to term and the index of every related 
    // assign() term
    std::map<Term*, std::vector<int> > termToAssignTerms;

    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];
        if (term->function == ASSIGN_FUNC) {
            Term* target = term->input(0);
            termToAssignTerms[target].push_back(i);
        }
    }

    // Then, iterate over assigned-to terms
    std::map<Term*, std::vector<int> >::const_iterator it;
    for (it = termToAssignTerms.begin(); it != termToAssignTerms.end(); ++it) {
        int assignCount = (int) it->second.size();
        if (assignCount > 1) {

            Term* target = it->first;

            // Remove all of the assign() terms, and make a list of terms to send
            // to the feedback-accumulator
            std::vector<int>::const_iterator index_it;
            RefList accumulatorInputs;

            for (index_it = it->second.begin(); index_it != it->second.end(); ++index_it) {
                int index = *index_it;
                assert(branch[index] != NULL);
                accumulatorInputs.append(branch[index]->input(0));
                branch[index] = NULL;
            }

            // Create a call to their feedback-accumulator
            // TODO: Should probably choose the accumulator func based on type
            Term* accumulator = apply(&branch, AVERAGE_FUNC, accumulatorInputs);

            // assign() this
            apply(&branch, ASSIGN_FUNC, RefList(accumulator, target));
        }
    }

    branch.removeNulls();
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

    normalize_training_branch(trainingBranch);
}

} // namespace circa
