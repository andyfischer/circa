// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

bool is_value_alloced(Term* term)
{
    if (term->type == NULL) {
        assert(term->value_data.ptr == NULL);
        return false;
    }

    return true;

#if 0
    if (!type_t::get_is_pointer(term->type))
        return true;
    else
        return term->value_data.ptr != NULL;
#endif
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

    term->value_type = &as_type(term->type);
}

void assign_value(Term* source, Term* dest)
{
    if (!is_value_alloced(source)) {
        assert(false);
        std::cout << "unsupported in assign_value" << std::endl;
        return;
    }

    // Do a type specialization if dest has type 'any'.
    // This might be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    Type::AssignFunc assign = type_t::get_assign_func(dest->type);

    if (assign == NULL) {
        assign_value((TaggedValue*) source, (TaggedValue*) dest);
        return;
    }

    // Deprecated behavior:
    if (!value_fits_type(source, dest->type)) {
        std::stringstream err;
        err << "In assign_value, element of type " << source->type->name <<
            " doesn't fit in type " << dest->type->name;
        throw std::runtime_error(err.str());
    }

    if (!is_value_alloced(dest))
        alloc_value(dest);


    assign(source, dest);
}

} // namespace circa
