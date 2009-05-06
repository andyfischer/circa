// Copyright 2009 Andrew Fischer

#include "circa.h"

namespace circa {

bool is_stateful(Term* term)
{
    return term->function == STATEFUL_VALUE_FUNC;
}

void set_stateful(Term* term, bool value)
{
    term->boolProperty("stateful") = value;
}

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
