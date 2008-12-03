// Copyright 2008 Paul Hodge

#ifndef CIRCA_REFERENCE_INCLUDED
#define CIRCA_REFERENCE_INCLUDED

#include "common_headers.h"

namespace circa {

struct Reference
{
    Term* _term;

    Reference() : _term(NULL) {}
    Reference(Term* t) : _term(NULL) { set(t); }
    Reference(Reference const& ref) : _term(NULL) { set(ref._term); }

    ~Reference();
    void set(Term* t);
    Term* get() const { return _term; }
    Reference& operator=(Term* t);
    Reference& operator=(Reference const& ref);
    Term& operator*() const;
    Term* operator->() const;
};

bool operator==(Reference const& lhs, Reference const& rhs);
bool operator<(Reference const& lhs, Reference const& rhs);

}

#endif
