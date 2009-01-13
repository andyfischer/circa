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
        _term = create_value(NULL, INT_TYPE);
        _owned = true;
    }
    Int(Branch &branch, std::string const& name="") {
        _term = create_value(&branch, INT_TYPE, NULL, name);
        _owned = false;
    }   
    Int(Term* term) {
        _term = term;
        _owned = false;
    }
    ~Int() {
        if (_owned)
            delete _term;
    }
    Int& operator=(int value)
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
