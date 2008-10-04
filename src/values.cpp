// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "circa.h"
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
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    // Don't steal if the term says not to
    // Also don't steal if the term has multiple users
    bool steal = !source->stealingOk && (source->users.count() > 1);

    if (steal)
        steal_value(source, dest);
    else
        duplicate_value(source, dest);
}

void duplicate_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    Type::DuplicateFunc duplicate = as_type(source->type)->duplicate;

    if (duplicate == NULL)
        throw errors::InternalError(std::string("type ") + as_type(source->type)->name
                + " has no duplicate function");

    dealloc_value(dest);

    duplicate(source, dest);
}

void steal_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    // if 'dest' has a value, delete it
    dealloc_value(dest);

    dest->value = source->value;

    source->value = NULL;
    source->needsUpdate = true;
}

} // namespace circa
