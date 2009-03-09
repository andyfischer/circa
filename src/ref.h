// Copyright 2008 Paul Hodge

#ifndef CIRCA_REF_INCLUDED
#define CIRCA_REF_INCLUDED

// Ref
//
// Maintains a safe weak pointer to a Term.

#include "common_headers.h"

namespace circa {

struct Ref
{
    Term* _t;
    Term* _owner;

    Ref()
      : _t(NULL),
        _owner(NULL)
    {
    }

    Ref(Term *initialValue)
      : _t(NULL),
        _owner(NULL)
    {
        set(initialValue);
    }

    // Copy constructor
    Ref(Ref const& copy)
      : _t(NULL),
        _owner(copy._owner)
    {
        set(copy._t);
    }

    ~Ref()
    {
        set(NULL);
    }

    // Assignment copy
    Ref& operator=(Ref const& rhs)
    {
        _owner = rhs._owner;
        set(rhs._t);
        return *this;
    }

    void set(Term* target);

    Ref& operator=(Term* target)
    {
        set(target);
        return *this;
    }

    bool operator==(Term* t) const
    {
        return _t == t;
    }

    operator Term*() const
    {
        return _t;
    }

    operator Term*&()
    {
        return _t;
    }

    Term* operator->()
    {
        return _t;
    }

    static void remap_pointers(Term* term, ReferenceMap const& map);
    static ReferenceIterator* start_reference_iterator(Term* term);
};

void remove_referencer(Term* term, Ref* ref);
void delete_term(Term* term);

} // namespace circa

#endif // CIRCA_REFERENCE_INCLUDED
