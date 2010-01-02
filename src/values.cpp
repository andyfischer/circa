// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

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

void assign_value(Term* source, Term* dest)
{
    if (!is_value_alloced(source)) {
        dealloc_value(dest);
        return;
    }

    // Do a type specialization if dest has type 'any'.
    // This might be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    if (!value_fits_type(source, dest->type)) {
        std::stringstream err;
        err << "In assign_value, element of type " << source->type->name <<
            " doesn't fit in type " << dest->type->name;
        throw std::runtime_error(err.str());
    }

    if (!is_value_alloced(dest))
        alloc_value(dest);

    AssignFunc assign = type_t::get_assign_func(dest->type);

    if (assign == NULL)
        throw std::runtime_error("type "+type_t::get_name(dest->type)+" has no assign function");

    assign(source, dest);
}

} // namespace circa
