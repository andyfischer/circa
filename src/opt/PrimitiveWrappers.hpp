// Copyright 2008 Paul Hodge

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
        if (name != "" && branch.containsName(name)) {
            _term = branch.findFirstBinding(name);

            assert(_term->type == INT_TYPE);

            _owned = false;
        } else {
            _term = create_value(&branch, INT_TYPE, name);
            _owned = false;
        }
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

class Float {
private:
    Term* _term;
    bool _owned;
public:
    Float() {
        _term = create_value(NULL, FLOAT_TYPE);
        _owned = true;
    }
    Float(Branch &branch, std::string const& name="") {
        if (name != "" && branch.containsName(name)) {
            _term = branch.findFirstBinding(name);

            assert(_term->type == FLOAT_TYPE);

            _owned = false;
        } else {
            _term = create_value(&branch, FLOAT_TYPE, name);
            _owned = false;
        }
    }   
    Float(Term* term) {
        _term = term;
        _owned = false;
    }
    ~Float() {
        if (_owned)
            delete _term;
    }
    Float& operator=(float value)
    {
        as_float(_term) = value;
        return *this;
    }
    operator float() const
    {
        return as_float(_term);
    }
};

class Bool {
private:
    Term* _term;
    bool _owned;
public:
    Bool() {
        _term = create_value(NULL, BOOL_TYPE);
        _owned = true;
    }
    Bool(Branch &branch, std::string const& name="") {
        if (name != "" && branch.containsName(name)) {
            _term = branch.findFirstBinding(name);

            assert(_term->type == BOOL_TYPE);

            _owned = false;
        } else {
            _term = create_value(&branch, BOOL_TYPE, name);
            _owned = false;
        }
    }   
    Bool(Term* term) {
        _term = term;
        _owned = false;
    }
    ~Bool() {
        if (_owned)
            delete _term;
    }
    Bool& operator=(bool value)
    {
        as_bool(_term) = value;
        return *this;
    }
    operator bool() const
    {
        return as_bool(_term);
    }
};

} // namespace circa

#endif
