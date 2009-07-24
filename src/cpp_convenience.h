// Copyright 2009 Paul Hodge

#ifndef CIRCA_CPP_CONVENIENCE_INCLUDED
#define CIRCA_CPP_CONVENIENCE_INCLUDED

#include "builtins.h"
#include "cpp_importing.h"
#include "refactoring.h"
#include "primitives.h"

namespace circa {

template <class T, Term** type, T& (accessorFunc)(Term*) >
class Accessor {
    Ref _term;

public:
    Accessor(Branch& branch, std::string const& name, T const& defaultValue)
    {
        if (branch.contains(name)) {
            _term = branch[name];

            if (_term->type != *type) {
                change_type(_term, *type);
                accessorFunc(_term) = defaultValue;
            }
        } else {
            _term = create_value(branch, *type, name);
            accessorFunc(_term) = defaultValue;
        }
    }

    Accessor& operator=(T const& rhs)
    {
        accessorFunc(_term) = rhs;
        return *this;
    }

    operator T&()
    {
        return accessorFunc(_term);
    }

    T& get()
    {
        return accessorFunc(_term);
    }
};

typedef Accessor<int, &INT_TYPE, as_int> Int;
typedef Accessor<float, &FLOAT_TYPE, as_float> Float;
typedef Accessor<bool, &BOOL_TYPE, as_bool> Bool;
typedef Accessor<std::string, &STRING_TYPE, as_string> String;

} // namespace circa

#endif
