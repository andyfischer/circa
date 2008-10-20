// Copyright 2008 Paul Hodge

#include "common_headers.h"
#include "circa.h"
#include "values.h"

namespace circa {

void dealloc_value(Term* term)
{
    if (term->value == NULL)
        return;

    if (term->type == NULL)
        return;

    if (term->type->value == NULL)
        throw std::runtime_error("type is undefined");

    if (as_type(term->type)->dealloc == NULL)
        throw std::runtime_error("type " + as_type(term->type)->name
            + " has no dealloc function");

    as_type(term->type)->dealloc(term);
    term->value = NULL;
}

void recycle_value(Term* source, Term* dest)
{
    assert_type(source, dest->type);

    // Only steal if the term says it's OK
    bool steal = source->stealingOk;

    // Don't steal from types
    if (source->type == TYPE_TYPE)
        steal = false;

    if (steal)
        steal_value(source, dest);
    else
        duplicate_value(source, dest);
}

void duplicate_value(Term* source, Term* dest)
{
    if (source == dest)
        throw std::runtime_error("in duplicate_value, can't have source == dest");

    assert_type(source, dest->type);

    Type::DuplicateFunc duplicate = as_type(source->type)->duplicate;

    if (duplicate == NULL)
        throw std::runtime_error(std::string("type ") + as_type(source->type)->name
                + " has no duplicate function");

    dealloc_value(dest);

    duplicate(source, dest);
}

void steal_value(Term* source, Term* dest)
{
    assert_type(source, dest->type);

    // if 'dest' has a value, delete it
    dealloc_value(dest);

    dest->value = source->value;

    source->value = NULL;
    source->needsUpdate = true;
}

Term* string_var(Branch& branch, std::string const& s, std::string const& name)
{
    Term* term = apply_function(branch, STRING_TYPE, ReferenceList());
    as_string(term) = s;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* int_var(Branch& branch, int i, std::string const& name)
{
    Term* term = apply_function(branch, INT_TYPE, ReferenceList());
    as_int(term) = i;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* float_var(Branch& branch, float f, std::string const& name)
{
    Term* term = apply_function(branch, FLOAT_TYPE, ReferenceList());
    as_float(term) = f;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* list_var(Branch& branch, ReferenceList list, std::string const& name)
{
    Term* term = apply_function(branch, LIST_TYPE, ReferenceList());
    // FIXME as_list(term) = list;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

} // namespace circa
