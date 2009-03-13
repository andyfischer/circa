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
        for (int i=0; i < it->numInputs(); i++) {
            if (is_trainable(it->input(i)))
                it->boolProperty("derived-trainable") = true;
        }
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
