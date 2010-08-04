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

    if (state != NULL) {
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
    make_list(stateTv);
    List* state = List::checkCast(stateTv);

    // Count the # of stateful terms in branch
    int statefulTerms = 0;
    for (int i=0; i < branch.length(); i++)
        if (is_stateful(branch[i]))
            statefulTerms++;

    state->resize(statefulTerms);

    int write = 0;
    for (int read=0; read < branch.length(); read++) {
        Term* term = branch[read];

        if (!is_stateful(term))
            continue;

        copy(term, state->get(write++));
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

TaggedValue* get_hidden_state_for_call(Term* term)
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
    TaggedValue state;
    persist_state_from_branch(source, &state);
    load_state_into_branch(&state, dest);
}

void reset_state(Branch& branch)
{
    for (BranchIterator it(branch); !it.finished(); ++it) {
        if (is_stateful(*it))
            reset(*it);
    }
}

} // namespace circa
