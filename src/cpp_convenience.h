// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_CPP_CONVENIENCE_INCLUDED
#define CIRCA_CPP_CONVENIENCE_INCLUDED

#include "builtins.h"
#include "cpp_importing.h"
#include "errors.h"
#include "refactoring.h"
#include "primitives.h"

namespace circa {

template <class T>
T& as(Term* term)
{
    Term* type = term->type;
    const std::type_info* type_info = type_t::get_std_type_info(term->type);

    if (type_info == NULL)
        throw std::runtime_error("No type info found for type "+type_t::get_name(type));

    if (*type_info != typeid(T))
        throw std::runtime_error(
                "C++ type mismatch, existing data has type "+type_t::get_name(type));

    return *((T*) term->value);
}

// Specializations for primitive types:
template <> int& as(Term* term);
template <> float& as(Term* term);
template <> bool& as(Term* term);
template <> std::string& as(Term* term);

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
            _term = create_value(branch, *type, name);
            as<T>(_term) = defaultValue;
        }
    }

    Accessor()
    {
    }

    void reset(Term* term)
    {
        _term = term;
    }

    Accessor& operator=(Term* term)
    {
        reset(term);
        return *this;
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

template <class T>
T eval(std::string const& statement)
{
    Branch branch;

    Term* result = branch.eval(statement);

    if (result->hasError())
        throw std::runtime_error(get_error_message(result));

    return as<T>(result);
}

template <class T>
T eval(Branch &branch, std::string const& statement)
{
    Term* result = branch.eval(statement);

    if (result->hasError())
        throw std::runtime_error(get_error_message(result));

    return as<T>(result);
}

} // namespace circa

#endif
