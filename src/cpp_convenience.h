// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_CPP_CONVENIENCE_INCLUDED
#define CIRCA_CPP_CONVENIENCE_INCLUDED

#include "builtins.h"
#include "cpp_importing.h"
#include "errors.h"
#include "refactoring.h"
#include "tagged_value_accessors.h"

namespace circa {

template <class T>
T as(Term* term)
{
    Type* type = &as_type(term->type);
    const std::type_info* type_info = type->cppTypeInfo;

    if (type_info == NULL)
        throw std::runtime_error("No type info found for type "+type->name);

    if (*type_info != typeid(T))
        throw std::runtime_error(
                "C++ type mismatch, existing data has type "+type->name);

    return *((T*) term->value_data.ptr);
}

class Int {
    TaggedValue* _value;

public:
    Int(TaggedValue* value) { _value = value; }
    
    Int& operator=(int rhs) { set_int(_value, rhs); return *this; }
    operator int() { return as_int(_value); }
};

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
