// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_REFERENCES_INCLUDED
#define CIRCA_REFERENCES_INCLUDED

#include "common_headers.h"

namespace circa {

struct Ref
{
    // Super short name for this field because we often need to type it into
    // the debugger.
    Term* t;

    Ref() : t(NULL) {}

    Ref(Term *initialValue) : t(NULL)
    {
        set(initialValue);
    }

    // Copy constructor
    Ref(Ref const& copy) : t(NULL)
    {
        set(copy.t);
    }

    ~Ref()
    {
        set(NULL);
    }

    // Assignment copy
    Ref& operator=(Ref const& rhs)
    {
        set(rhs.t);
        return *this;
    }

    void set(Term* target);

    Ref& operator=(Term* target)
    {
        set(target);
        return *this;
    }

    bool operator==(Term* _t) const
    {
        return _t == t;
    }

    operator Term*() const
    {
        return t;
    }

    Term* operator->()
    {
        return t;
    }

    static void remap_pointers(Term* term, ReferenceMap const& map);
    static ReferenceIterator* start_reference_iterator(Term* term);
};

void delete_term(Term* term);
void remap_pointers(Term* term, ReferenceMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);
void remap_pointers(Branch& branch, Term* original, Term* replacement);

} // namespace circa

#endif // CIRCA_REFERENCE_INCLUDED
