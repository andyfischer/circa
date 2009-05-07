// Copyright 2009 Andrew Fischer

#include "circa.h"

namespace circa {

bool is_stateful(Term* term)
{
    return term->boolPropOptional("stateful", false);
}

void set_stateful(Term* term, bool value)
{
    term->boolProp("stateful") = value;
}

void load_state_into_branch(Branch& state, Branch& branch)
{
    int read = 0;

    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];

        if (!is_stateful(term))
            continue;

        if (read > state.numTerms())
            return;

        assign_value(state[read++], branch[i]);
    }
}

void persist_state_from_branch(Branch& branch, Branch& state)
{
    state.clear();

    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];

        if (!is_stateful(term))
            continue;

        Term* state_dup = create_value(&state, term->type, term->name);
        assign_value(term, state_dup);
    }
}

void get_type_from_branches_stateful_terms(Branch& branch, Branch& type)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];

        if (!is_stateful(term))
            continue;

        create_value(&type, term->type, term->name);
    }
}

bool has_hidden_state(Function& func)
{
    return func.hiddenStateType != NULL && func.hiddenStateType != VOID_TYPE;
}

Term* get_hidden_state_for_call(Term* term)
{
    if (has_hidden_state(as_function(term->function)))
        return term->input(0);
    else
        return NULL;
}

} // namespace circa
