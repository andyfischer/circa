// Copyright 2009 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {

bool is_trainable(Term* term)
{
    return term->boolPropertyOptional("trainable", false)
        || term->boolPropertyOptional("derived-trainable", false);
}

void update_derived_trainables(Branch& branch)
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

} // namespace circa
