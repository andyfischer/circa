// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

bool is_value_alloced(Term* term)
{
    if (term->type == NULL) {
        assert(term->value.data.ptr == NULL);
        return false;
    }

    if (!type_t::get_is_pointer(term->type))
        return true;
    else
        return term->value.data.ptr != NULL;
}

void alloc_value(Term* term)
{
    if (is_value_alloced(term)) return;

    Type::AllocFunc alloc = type_t::get_alloc_func(term->type);

    if (alloc != NULL) {
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

    Type::DeallocFunc dealloc = type_t::get_dealloc_func(term->type);

    if (!is_value_alloced(term->type)) {
        std::cout << "warn: in dealloc_value, type is undefined" << std::endl;
        set_null(term->value);
        return;
    }

    if (dealloc != NULL)
        dealloc(term->type, term);

    set_null(term->value);
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

    Type::AssignFunc assign = type_t::get_assign_func(dest->type);

    if (assign == NULL) {
        assign_value(source->value, dest->value);
        return;
    }

    assign(source, dest);
}

} // namespace circa
