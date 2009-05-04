// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {

void load_state_into_branch(Term* state, Branch& branch)
{
    Branch& stateBranch = as_branch(state);

    for (int i=0; i < stateBranch.numTerms(); i++) {
        Term* term = stateBranch[i];

        std::string name = term->name;

        if (name == "")
            continue;

        Term* destination = branch.findFirstBinding(name);

        assign_value(term, destination);
    }
}

void persist_state_from_branch(Branch& branch, Term* state)
{
    Branch& stateBranch = as_branch(state);

    for (int i=0; i < stateBranch.numTerms(); i++) {
        Term* term = stateBranch[i];

        std::string name = term->name;

        if (name == "")
            continue;

        Term* source = branch[name];

        assign_value(source, term);
    }
}

} // namespace circa
