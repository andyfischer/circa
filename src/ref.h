// Copyright 2008 Paul Hodge

#ifndef CIRCA_REF_INCLUDED
#define CIRCA_REF_INCLUDED

// Ref
//
// Maintains a safe weak pointer to a Term.

#include "common_headers.h"

#include "referencer.h"

namespace circa {

struct Ref : public Referencer
{
    Term* _target;
    Term* _owner;

    Ref()
      : _target(NULL),
        _owner(NULL)
    {
    }

    Ref(Term *initialValue)
      : _target(initialValue),
        _owner(NULL)
    {
    }

    // Copy constructor
    Ref(Ref const& copy)
      : _target(NULL),
        _owner(copy._owner)
    {
        set(copy._target);
    }

    ~Ref()
    {
        set(NULL);
    }

    // Assignment copy
    Ref& operator=(Ref const& rhs)
    {
        _owner = rhs._owner;
        set(rhs._target);
        return *this;
    }

    void set(Term* target);

    // Convenience = overload
    Ref& operator=(Term* target)
    {
        set(target);
        return *this;
    }

    operator Term*()
    {
        return _target;
    }

    Term* operator->()
    {
        return _target;
    }
};

} // namespace circa

#endif // CIRCA_REFERENCE_INCLUDED
