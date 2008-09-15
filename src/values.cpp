// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "essentials.h"
#include "values.h"

namespace circa {

namespace values_private {

void really_steal_value(Term* source, Term* dest)
{
    // if 'dest' has a value, delete it
    dealloc_value(dest);

    dest->value = source->value;

    source->value = NULL;
    source->needsUpdate = true;
}

} // namespace values_private

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
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    // Don't steal if the term has multiple users
    bool steal = (source->users.count() > 1);

    // Temp: always try to steal
    steal_value(source, dest);

    // Recursively call for all fields
    int numFields = as_type(source->type)->numFields();
    for (int fieldIndex=0; fieldIndex < numFields; fieldIndex++) {
        recycle_value(source->fields[fieldIndex], dest->fields[fieldIndex]);
    }
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
}

void steal_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    // Don't steal from constant terms
    if (is_constant(source)) {
        duplicate_value(source, dest);
        return;
    }

    values_private::really_steal_value(source, dest);
}

} // namespace circa
