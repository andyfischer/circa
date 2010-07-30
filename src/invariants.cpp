// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "builtins.h"
#include "term.h"
#include "type.h"

namespace circa {

bool check_invariants(Term* term, std::string& result)
{
    if (term->value_type == NULL) {
        result = "TaggedValue has null type";
        ca_assert(false);
        return false;
    }

    if (term->type != NULL
            && (term->value_type != &as_type(term->type))
            && term->type != ANY_TYPE
            && !is_branch(term)) {
        result = "Value has wrong type: term->type is " + term->type->name + ", tag is "
            + term->value_type->name;
        ca_assert(false);
        return false;
    }

    if (term->nestedContents.owningTerm != term) {
        result = "Term.nestedContents has wrong owningTerm";
        return false;
    }

    if (term->owningBranch != NULL) {
        Branch& branch = *term->owningBranch;
        if ((term->index >= branch.length())
                || (branch[term->index] != term)) {
            result = "Term.index doesn't resolve to this term in owningBranch";
            return false;
        }
    }

    return true;
}

} // namespace circa
