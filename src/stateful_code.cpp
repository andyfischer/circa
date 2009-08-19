// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

bool MIGRATE_STATEFUL_VALUES_VERBOSE = false;

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
    assert(is_function(func));
    Term* stateType = function_t::get_hidden_state_type(func);
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

        create_value(type, term->type, term->name);
    }
}

Term* get_hidden_state_for_call(Term* term)
{
    if (is_function_stateful(term->function)) {
        assert(term->input(0) != NULL);
        return term->input(0);
    } else
        return NULL;
}

bool terms_match_for_migration(Term* left, Term* right)
{
    if (left->name != right->name) {
        if (MIGRATE_STATEFUL_VALUES_VERBOSE)
            std::cout << "reject, names don't match" << std::endl;
        return false;
    }

    bool typesFit = left->type == right->type;

    if (!typesFit)
        typesFit = value_fits_type(left, right->type);
      
    if (!typesFit) {
        if (MIGRATE_STATEFUL_VALUES_VERBOSE)
            std::cout << "reject, types aren't equal" << std::endl;
        return false;
    }

    return true;
}

void migrate_stateful_values(Branch& source, Branch& dest)
{
    // There are a lot of fancy algorithms we could do here. But for now, just
    // iterate and check matching indexes.
    
    for (int index=0; index < source.length(); index++) {
        if (MIGRATE_STATEFUL_VALUES_VERBOSE)
            std::cout << "checking index: " << index << std::endl;

        if (index >= dest.length())
            break;

        Term* sourceTerm = source[index];
        Term* destTerm = dest[index];

        if (sourceTerm == NULL || destTerm == NULL)
            continue;

        if (!terms_match_for_migration(sourceTerm, destTerm))
            continue;

        // At this point, they match
        
        // If both terms are subroutine calls, and the source call is expanded, then
        // expand the dest call as well.
        if (is_subroutine(sourceTerm->function) && is_subroutine(destTerm->function))
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

        // Migrate inner branches (but not branches that should be treated as values)
        if (is_branch(sourceTerm) && !is_stateful(sourceTerm)
                && is_branch(destTerm)) {
            migrate_stateful_values(as_branch(sourceTerm), as_branch(destTerm));
        } 
        
        // Stateful value migration
        else if (is_stateful(sourceTerm)
                    && is_stateful(destTerm)
                    && is_value_alloced(sourceTerm)) {

            if (MIGRATE_STATEFUL_VALUES_VERBOSE)
                std::cout << "assigning value of " << to_string(sourceTerm) << std::endl;

            assign_value(sourceTerm, destTerm);
        } else {
            if (MIGRATE_STATEFUL_VALUES_VERBOSE)
                std::cout << "nothing to do" << std::endl;
        }
    }
}

void reset_state(Branch& branch)
{
    for (BranchIterator it(branch); !it.finished(); ++it)
        if (is_stateful(*it))
            assign_value_to_default(*it);
}

} // namespace circa
