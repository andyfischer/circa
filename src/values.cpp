// Copyright 2008 Paul Hodge

#include "common_headers.h"
#include "essentials.h"
#include "values.h"

namespace circa {

void dealloc_value(Term* term)
{
    if (term->value == NULL)
        return;

    if (as_type(term->type)->dealloc == NULL)
        throw errors::InternalError("type " + as_type(term->type)->name
            + " has no dealloc function");

    as_type(term->type)->dealloc(term);
    term->value = NULL;
}

void recycle_value(Term* source, Term* dest)
{
    // Don't steal if the term has multiple users
    bool steal = (source->users.count() > 1);

    // Temp: always try to steal
    steal_value(source, dest);
}

void duplicate_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    Type::DuplicateFunc duplicate = as_type(source->type)->duplicate;

    if (duplicate == NULL)
        throw errors::InternalError(string("type ") + as_type(source->type)->name
                + " has no duplicate function");

    dealloc_value(dest);

    duplicate(source, dest);

    // duplicate fields too
    for (int i=0; i < source->fields.size(); i++) {
        duplicate_value(source->fields[i], dest->fields[i]);
    }
}

void steal_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    // In some situations, ignore their request to steal

    // Don't steal from constant terms
    if (is_constant(source)) {
        duplicate_value(source, dest);
        return;
    }

    // if 'dest' has a value, delete it
    dealloc_value(dest);

    dest->value = source->value;

    // steal fields as well
    TermNamespace::iterator it;
    for (int i=0; i < source->fields.size(); i++) {
        steal_value(source->fields[i], dest->fields[i]);
    }

    source->value = NULL;
    source->needsUpdate = true;
}

} // namespace circa
