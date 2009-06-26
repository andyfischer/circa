// Copyright 2009 Paul Hodge

#ifndef CIRCA_CPP_CONVENIENCE_INCLUDED
#define CIRCA_CPP_CONVENIENCE_INCLUDED

#include "builtins.h"
#include "cpp_importing.h"
#include "refactoring.h"
#include "primitives.h"

namespace circa {

template <class T, Term** type>
class Accessor {
    Ref _term;

public:
    Accessor(Branch& branch, std::string const& name, T const& defaultValue)
    {
        if (branch.contains(name)) {
            _term = branch[name];

            if (_term->type != *type) {
                change_type(_term, *type);
                as<T>(_term) = defaultValue;
            }
        } else {
            _term = create_value(&branch, *type, name);
            as<T>(_term) = defaultValue;
        }
    }

    Accessor& operator=(T const& rhs)
    {
        as<T>(_term) = rhs;
        return *this;
    }

    operator T&()
    {
        return as<T>(_term);
    }

    T& get()
    {
        return as<T>(_term);
    }
};

typedef Accessor<int, &INT_TYPE> Int;
typedef Accessor<float, &FLOAT_TYPE> Float;
typedef Accessor<bool, &BOOL_TYPE> Bool;
typedef Accessor<std::string, &STRING_TYPE> String;

} // namespace circa

#endif
