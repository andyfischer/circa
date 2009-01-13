// Copyright 2008 Andrew Fischer

#ifndef CIRCA_PRIMITIVE_WRAPPERS_INCLUDED
#define CIRCA_PRIMITIVE_WRAPPERS_INCLUDED

// This file should not be included by Circa code. It's optional.

#include "circa.h"

namespace circa {

class Int {
private:
    Term* _term;
    bool _owned;
public:
    Int() {
        _term = create_term(NULL, INT_TYPE);
        _owned = true;
    }
    Int(Branch &branch) {
        _term = create_term(branch, INT_TYPE);
        _owned = true;
    }   
    Int(Term* term) {
        _term = term;
        _owned = false;
    }
    ~Int() {
        if (_owned)
            delete _term;
    }
    Int& operator=(int value);
    {
        as_int(_term) = value;
        return *this;
    }
    operator int() const
    {
        return as_int(_term);
    }
};

}

#endif
