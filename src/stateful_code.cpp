// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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
    Term* stateType = function_t::get_inline_state_type(func);
    return (stateType != NULL && stateType != VOID_TYPE);
}

bool has_any_inlined_state(Branch& branch)
{
    for (int i=0; i < branch.length(); i++) {
        if (is_get_state(branch[i]))
            return true;
        if (has_implicit_state(branch[i]))
            return true;
    }
    return false;
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
