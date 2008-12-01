// Copyright 2008 Paul Hodge

#ifndef CIRCA_REFERENCE_INCLUDED
#define CIRCA_REFERENCE_INCLUDED

#include "common_headers.h"

#include "term.h"

namespace circa {

struct Reference
{
    Term* term;

    Reference() : term(NULL) {}
    Reference(Term* t) : term(NULL)
    {
        set(t);
    }

    ~Reference()
    {
        if (term != NULL)
            term->references--;
    }
    
    void set(Term* t)
    {
        if (term == t)
            return;

        if (term != NULL) {
            term->references--;
        }

        term = t;
        term->references++;
    }

    Reference& operator=(Term* t)
    {
        set(t);
        return *this;
    }
};

}

#endif
