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

bool is_function_stateful(Term* func)
{
    Term* stateType = as_function(func).hiddenStateType;
    return (stateType != NULL && stateType != VOID_TYPE);
}

void load_state_into_branch(Branch& state, Branch& branch)
{
    int read = 0;

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (!is_stateful(term))
            continue;

        if (read >= state.length())
            return;

        assign_value(state[read++], term);
    }
}

void persist_state_from_branch(Branch& branch, Branch& state)
{
    int write = 0;
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (!is_stateful(term))
            continue;

        rewrite_as_value(state, write, term->type);
        assign_value(term, state[write++]);
    }
}

void get_type_from_branches_stateful_terms(Branch& branch, Branch& type)
{
    for (int i=0; i < branch.length(); i++) {
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
