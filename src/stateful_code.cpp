// Copyright 2009 Paul Hodge

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

bool function_has_hidden_state(Term* func)
{
    Term* hiddenStateType = get_function_data(func).hiddenStateType;
    return hiddenStateType != NULL && hiddenStateType != VOID_TYPE;
}

Term* get_hidden_state_for_call(Term* term)
{
    if (function_has_hidden_state(term->function)) {
        assert(term->input(0) != NULL);
        return term->input(0);
    } else
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

        // If both terms are subroutine calls, and the source call is expanded, then
        // expand the dest call as well.
        if ((sourceTerm->function->type == SUBROUTINE_TYPE)
                && (destTerm->function->type == SUBROUTINE_TYPE))
        {
            Term* sourceCallState = get_hidden_state_for_call(sourceTerm);
            Term* destCallState = get_hidden_state_for_call(destTerm);
            if (sourceCallState != NULL && destCallState != NULL) {
                if (is_subroutine_state_expanded(sourceCallState)
                    && !is_subroutine_state_expanded(destCallState))
                    expand_subroutines_hidden_state(destTerm, destCallState);

                // The loop just passed over these terms, so call migrate on them again.
                migrate_stateful_values(as_branch(sourceCallState), as_branch(destCallState));
            }
        }

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
