// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

#if 0
bool share_value(Term* original, Term* share)
{
    assert(value_fits_type(share, original->type));

    dealloc_value(share);
    share->flags = share->flags | TERM_FLAG_SHARED_VALUE;
    share->value = original->value;
}
#endif

bool is_value_alloced(Term* term)
{
    if (term->type == NULL) {
        assert(term->value == NULL);
        return false;
    }

    if (!type_t::get_is_pointer(term->type))
        return true;
    else
        return term->value != NULL;
}

void alloc_value(Term* term)
{
    if (is_value_alloced(term)) return;

    AllocFunc alloc = type_t::get_alloc_func(term->type);

    if (alloc == NULL)
        // this happens while bootstrapping
        term->value = NULL;
    else {
        alloc(term->type, term);

        if (term->type != TYPE_TYPE)
            assign_value_to_default(term);

        if (is_branch(term))
            as_branch(term).owningTerm = term;
    }
}

void dealloc_value(Term* term)
{
    if (term->type == NULL) return;
    if (!is_value_alloced(term)) return;

    DeallocFunc dealloc = type_t::get_dealloc_func(term->type);

    if (!is_value_alloced(term->type)) {
        std::cout << "warn: in dealloc_value, type is undefined" << std::endl;
        term->value = NULL;
        return;
    }

    if (dealloc != NULL)
        dealloc(term->type, term);

    term->value = NULL;
}

} // namespace circa
