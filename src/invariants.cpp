// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "builtins.h"
#include "term.h"
#include "type.h"

namespace circa {

bool check_invariants(Term* term, std::string& result)
{
    if (term->value_type == NULL) {
        result = "TaggedValue has null type";
        return false;
    }

    if (term->type != NULL) {

        bool typeOk = (term->type == ANY_TYPE)
            || (term->type == VOID_TYPE && is_null(term))
            || cast_possible(term, unbox_type(term->type));

        if (!typeOk) {
            result = "Value has wrong type: term->type is " + term->type->name + ", tag is "
                + term->value_type->name;
            return false;
        }
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
