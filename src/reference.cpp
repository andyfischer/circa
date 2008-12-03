// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "reference.h"
#include "term.h"

namespace circa {

Reference::~Reference()
{
    if (_term != NULL)
        _term->references--;
}

void
Reference::set(Term* t)
{
    if (_term == t)
        return;

    if (_term != NULL)
        _term->references--;

    _term = t;

    if (_term != NULL)
        _term->references++;
}

Reference&
Reference::operator=(Term* t)
{
    set(t);
    return *this;
}

Reference&
Reference::operator=(Reference const& ref)
{
    set(ref._term);
    return *this;
}

Term&
Reference::operator*() const
{
    assert(_term != NULL);
    return *_term;
}

Term*
Reference::operator->() const
{
    assert(_term != NULL);
    return _term;
}

bool operator==(Reference const& lhs, Reference const& rhs)
{
    return lhs._term == rhs._term;
}

bool operator<(Reference const& lhs, Reference const& rhs)
{
    return lhs._term < rhs._term;
}

}
