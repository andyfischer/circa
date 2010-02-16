// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void assign_value(Term* source, Term* dest)
{
    // Do a type specialization if dest has type 'any'.
    // This might be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    Type::AssignFunc assign = as_type(dest->type).assign;

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

    assign(source, dest);
}

} // namespace circa
