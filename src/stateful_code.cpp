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

bool _terms_match_for_migration(Term* left, Term* right)
{
    if ((left->name == right->name) && (left->type == right->type))
        return true;

    return false;
}

void migrate_stateful_values(Branch& source, Branch& dest)
{
    // Figure out the mapping of source terms to dest terms
    std::map<Term*,Term*> map;

    // There are a lot of fancy algorithms we could do here. But for now, just
    // iterate and check matching indexes.
    
    for (int index=0; index < source.length(); index++) {
        if (index >= dest.length())
            break;

        Term* sourceTerm = source[index];
        Term* destTerm = dest[index];

        if (sourceTerm == NULL || destTerm == NULL)
            continue;

        if (!_terms_match_for_migration(sourceTerm, destTerm))
            continue;

        // At this point, they match

        // Migrate inner branches
        if (is_branch(sourceTerm)) {
            assert(is_branch(destTerm));
            migrate_stateful_values(as_branch(sourceTerm), as_branch(destTerm));
        } 
        
        // Stateful value migration
        else if (is_stateful(sourceTerm) && is_stateful(destTerm)) {
            assign_value(sourceTerm, destTerm);
        }
    }
}

} // namespace circa
