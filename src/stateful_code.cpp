// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

#define VERBOSE_LOG(f, ...) ; // disabled
//#define VERBOSE_LOG printf

bool subroutines_match_for_migration(Term* leftFunc, Term* rightFunc);

bool is_stateful(Term* term)
{
    return term->function == STATEFUL_VALUE_FUNC;
}

bool is_function_stateful(Term* func)
{
    if (!is_function(func))
        return false;
    Term* stateType = function_t::get_implicit_state_type(func);
    return (stateType != NULL && stateType != VOID_TYPE);
}

void load_state_into_branch(TaggedValue* stateTv, Branch& branch)
{
    int read = 0;
    int write = 0;

    List* state = List::checkCast(stateTv);

    for (write = 0; write < branch.length(); write++) {
        Term* destTerm = branch[write];

        if (!is_stateful(destTerm))
            continue;

        if (read >= state->length())
            break;

        if (!value_fits_type(state->get(read), type_contents(destTerm->type))) {
            reset(destTerm);
            break;
        }

        cast(state->get(read), destTerm);

        read++;
    }

    // if there are remaining stateful terms in 'branch' which didn't get
    // assigned, reset them.

    for (; write < branch.length(); write++) {
        if (is_stateful(branch[write]))
            reset(branch[write]);
    }
}

void persist_state_from_branch(Branch& branch, TaggedValue* stateTv)
{
    List* state = List::checkCast(stateTv);
    touch(state);

    int write = 0;
    for (int read=0; read < branch.length(); read++) {
        Term* term = branch[read];

        if (!is_stateful(term))
            continue;

        //rewrite_as_value(state, write, term->type);
        //rename(state[write], term->name);

        copy(term, state->get(write));

        write++;
    }

    if (write > state.length())
        state->resize(write);
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
    if (term->input(0) == NULL)
        return NULL;

    if (!is_function_stateful(term->function))
        return NULL;

    return term->input(0);
}

Term* find_call_for_hidden_state(Term* term)
{
    if (term->owningBranch == NULL)
        return NULL;

    Branch& branch = *term->owningBranch;

    Term* adjacent = branch[branch.getIndex(term)+1];

    return adjacent;
}

bool term_types_match_for_migration(Term* left, Term* right)
{
    //if (list_t::is_list_based_type(type_contents(left->type))
    //        && list_t::is_list_based_type(type_contents(right->type)))
    //    return true;

    if (!is_subtype(type_contents(right->type), left->value_type)) {
        VERBOSE_LOG("reject, types aren't equal\n");
        return false;
    }

    return true;
}

bool functions_match_for_migration(Term* left, Term* right)
{
    if (left->function->name != right->function->name)
        return false;

    return true;
}

bool terms_match_for_migration(Term* left, Term* right)
{
    if (left->name != right->name) {
        VERBOSE_LOG("reject, names don't match\n");
        return false;
    }

    if (!term_types_match_for_migration(left, right))
        return false;

    Term* leftCall = find_call_for_hidden_state(left);
    Term* rightCall = find_call_for_hidden_state(right);

    if (leftCall != NULL && rightCall != NULL
            && !functions_match_for_migration(leftCall, rightCall)) {
        VERBOSE_LOG("reject, assoiciated calls have mismatched functions\n");
        return false;
    }

    return true;
}

void mark_stateful_value_assigned(Term* term)
{
    // Check if this term has a "do once" block for assigning
    if (term->owningBranch == NULL) return;
    Branch* branch = term->owningBranch;
    if (branch->length() < int(term->index + 2)) return;
    Term* followingTerm = branch->get(term->index+1);
    Term* secondTerm = branch->get(term->index+2);
    if (followingTerm->function != STATEFUL_VALUE_FUNC) return;
    if (secondTerm->function != DO_ONCE_FUNC) return;
    Branch& doOnceBranch = secondTerm->nestedContents;
    if (doOnceBranch.length() == 0) return;
    Term* assignTerm = doOnceBranch[doOnceBranch.length()-1];
    if (assignTerm->function != UNSAFE_ASSIGN_FUNC) return;
    if (assignTerm->input(0) != term) return;

    Term* doOnceHiddenState = followingTerm;
    set_bool(doOnceHiddenState, true);
}

bool subroutines_match_for_migration(Term* leftFunc, Term* rightFunc)
{
    if (!is_subroutine(leftFunc)) return false;
    if (!is_subroutine(rightFunc)) return false;

    if (leftFunc->name != rightFunc->name)
        return false;

    return true;
}

void migrate_stateful_values(Branch& source, Branch& dest)
{
    // There are a lot of fancy algorithms we could do here. But for now, just
    // iterate and check matching indexes.
    
    for (int index=0; index < source.length(); index++) {
        VERBOSE_LOG("checking index: %d\n", index);

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
        if (subroutines_match_for_migration(sourceTerm->function, destTerm->function))
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
        if (sourceTerm->nestedContents.length() > 0
                && destTerm->nestedContents.length() > 0
                && !is_stateful(sourceTerm)) {
            migrate_stateful_values(sourceTerm->nestedContents, destTerm->nestedContents);
        } 
        
        // Stateful value migration
        else if (is_stateful(sourceTerm)
                    && is_stateful(destTerm)) {

            VERBOSE_LOG("assigning value of %s\n", to_string(sourceTerm).c_str());

            cast(sourceTerm, destTerm);
        } else {
            VERBOSE_LOG("nothing to do\n");
        }
    }
}

void reset_state(Branch& branch)
{
    for (BranchIterator it(branch); !it.finished(); ++it) {
        if (is_stateful(*it)) {
            reset(*it);
        }
    }
}

} // namespace circa
