// Copyright 2008 Paul Hodge

#ifndef CIRCA_PRIMITIVE_WRAPPERS_INCLUDED
#define CIRCA_PRIMITIVE_WRAPPERS_INCLUDED

#include "circa.h"

namespace circa {

class Int {
private:
    Ref _term;
public:
    Int() {
        _term = create_value(NULL, INT_TYPE);
    }
    Int(Branch &branch, std::string const& name="") {
        if (name != "" && branch.contains(name)) {
            _term = branch.findFirstBinding(name);

            assert(_term->type == INT_TYPE);
        } else {
            _term = create_value(&branch, INT_TYPE, name);
        }
    }   
    Int(Term* term) {
        _term = term;
    }
    ~Int() { }
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
    Ref _term;
public:
    Float() {
        _term = create_value(NULL, FLOAT_TYPE);
    }
    Float(Branch &branch, std::string const& name="") {
        if (name != "" && branch.contains(name)) {
            _term = branch.findFirstBinding(name);

            assert(_term->type == FLOAT_TYPE);
        } else {
            _term = create_value(&branch, FLOAT_TYPE, name);
        }
    }   
    Float(Term* term) {
        _term = term;
    }
    ~Float() { }
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
    Ref _term;
public:
    Bool() {
        _term = create_value(NULL, BOOL_TYPE);
    }
    Bool(Branch &branch, std::string const& name="") {
        if (name != "" && branch.contains(name)) {
            _term = branch.findFirstBinding(name);

            assert(_term->type == BOOL_TYPE);
        } else {
            _term = create_value(&branch, BOOL_TYPE, name);
        }
    }   
    Bool(Term* term) {
        _term = term;
    }
    ~Bool() { }
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
