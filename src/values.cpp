// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "circa.h"
#include "values.h"

namespace circa {

void alloc_value(Term* term)
{
    term->value = alloc_from_type(term->type);
    update_owner(term);
}

void dealloc_value(Term* term)
{
    if (term->value == NULL)
        return;

    if (term->type == NULL)
        return;

    if (term->type->value == NULL)
        throw std::runtime_error("type is undefined");

    if (term->ownsValue) {
        if (as_type(term->type).dealloc == NULL)
            throw std::runtime_error("type " + as_type(term->type).name
                + " has no dealloc function");

        as_type(term->type).dealloc(term->value);
    }

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

    Type::DuplicateFunc duplicate = as_type(source->type).duplicate;

    if (duplicate == NULL)
        throw std::runtime_error(std::string("type ") + as_type(source->type).name
                + " has no duplicate function");

    dealloc_value(dest);

    duplicate(source, dest);

    update_owner(dest);
}

void steal_value(Term* source, Term* dest)
{
    assert_type(source, dest->type);

    // if 'dest' has a value, delete it
    dealloc_value(dest);

    dest->value = source->value;

    source->value = NULL;
    source->needsUpdate = true;

    update_owner(dest);
}

void update_owner(Term* term)
{
    if (term->value == NULL)
        return;

    Type &type = as_type(term->type);

    if (type.updateOwner == NULL)
        return;

    type.updateOwner(term);
}

bool values_equal(Term* a, Term* b)
{
    if (a->type != b->type)
        return false;

    return as_type(a->type).equals(a,b);
}

Term* string_value(Branch& branch, std::string const& s, std::string const& name)
{
    Term* term = apply_function(branch, STRING_TYPE, ReferenceList());
    as_string(term) = s;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* int_value(Branch& branch, int i, std::string const& name)
{
    Term* term = apply_function(branch, INT_TYPE, ReferenceList());
    as_int(term) = i;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* float_value(Branch& branch, float f, std::string const& name)
{
    Term* term = apply_function(branch, FLOAT_TYPE, ReferenceList());
    as_float(term) = f;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* create_alias(Branch& branch, Term* term)
{
    return eval_function(branch, ALIAS_FUNC, ReferenceList(term));
}

} // namespace circa
