// Copyright 2008 Paul Hodge

#ifndef CIRCA_REFERENCE_INCLUDED
#define CIRCA_REFERENCE_INCLUDED

#include "common_headers.h"

#include "term.h"

namespace circa {

struct Reference
{
    Term* _term;

    Reference() : _term(NULL) {}
    Reference(Term* t) : _term(NULL) { set(t); }
    Reference(Reference const& ref) : _term(NULL) { set(ref._term); }

    ~Reference()
    {
        if (_term != NULL)
            _term->references--;
    }
    
    void set(Term* t)
    {
        if (_term == t)
            return;

        if (_term != NULL)
            _term->references--;

        _term = t;

        if (_term != NULL)
            _term->references++;
    }

    Reference& operator=(Term* t)
    {
        set(t);
        return *this;
    }
};

}

#endif
