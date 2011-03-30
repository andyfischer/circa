// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "function.h"
#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "stateful_code.h"
#include "term.h"

namespace circa {

bool subroutines_match_for_migration(Term* leftFunc, Term* rightFunc);

bool is_get_state(Term* term)
{
    return term->function->name == "get_state_field";
}

bool has_implicit_state(Term* term)
{
    if (is_function_stateful(term->function))
        return true;
    if (has_any_inlined_state(term->nestedContents))
        return true;
    return false;
}

bool is_function_stateful(Term* func)
{
    if (!is_function(func))
        return false;
    Term* stateType = get_function_attrs(func)->implicitStateType;
    return (stateType != NULL && stateType != VOID_TYPE);
}

bool has_any_inlined_state(Branch& branch)
{
    // Check if branch.hasInlinedState has a valid value.
    if (is_bool(&branch.hasInlinedState))
        return as_bool(&branch.hasInlinedState);

    // No valid value, recalculate.
    bool result = false;
    for (int i=0; i < branch.length(); i++) {
        if (is_get_state(branch[i])) {
            result = true;
            break;
        }

        if (has_implicit_state(branch[i])) {
            result = true;
            break;
        }
    }

    set_bool(&branch.hasInlinedState, result);
    return result;
}

void mark_branch_as_having_inlined_state(Branch& branch)
{
    if (is_bool(&branch.hasInlinedState) && as_bool(&branch.hasInlinedState))
        return;

    set_bool(&branch.hasInlinedState, true);
    Branch* parent = get_parent_branch(branch);
    if (parent != NULL)
        mark_branch_as_having_inlined_state(*parent);
}

void mark_branch_as_possibly_not_having_inlined_state(Branch& branch)
{
    if (is_null(&branch.hasInlinedState))
        return;

    set_null(&branch.hasInlinedState);
    Branch* parent = get_parent_branch(branch);
    if (parent != NULL)
        mark_branch_as_possibly_not_having_inlined_state(*parent);
}

const char* get_implicit_state_name(Term* term)
{
    return term->uniqueName.name.c_str();
}

void get_type_from_branches_stateful_terms(Branch& branch, Branch& type)
{
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (!is_get_state(term))
            continue;

        create_value(type, term->type, term->name);
    }
}

} // namespace circa
