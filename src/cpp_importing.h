// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_CPP_INTERFACE_INCLUDED
#define CIRCA_CPP_INTERFACE_INCLUDED

#include "importing.h"
#include "term.h"
#include "type.h"

namespace circa {
namespace cpp_importing {

template <class T>
void templated_initialize(Type* type, TaggedValue* value)
{
    // Use malloc() and memset(), this allows us to make sure that the value's memory
    // is zeroed out before it is used.
    void* data = malloc(sizeof(T));
    memset(data, 0, sizeof(T));
    new(data) T();
    value->value_data.ptr = data;
}

template <class T>
void templated_destroy(TaggedValue* value)
{
    // Placement delete because we used placement new above.
    reinterpret_cast<T*>(value->value_data.ptr)->~T();
    free((void*) value->value_data.ptr);
    value->value_data.ptr = 0;
}

template <class T>
void templated_assign(TaggedValue* source, TaggedValue* dest)
{
    *reinterpret_cast<T*>(dest->value_data.ptr)
        = *reinterpret_cast<T*>(source->value_data.ptr);
}

template <class T>
bool templated_equals(Term* a, Term* b)
{
    return (*(reinterpret_cast<T*>(a->value_data.ptr)))
        == (*(reinterpret_cast<T*>(b->value_data.ptr)));
}

template <class T>
bool templated_lessThan(Term* a, Term* b)
{
    return (*(reinterpret_cast<T*>(a->value_data.ptr)))
        < (*(reinterpret_cast<T*>(b->value_data.ptr)));
}

bool raw_value_less_than(Term* a, Term* b);

} // namespace cpp_importing

// Public functions

template <class T>
void import_type(Term* term)
{
    Type* type = &as_type(term);
    reset_type(type);

    type->initialize = cpp_importing::templated_initialize<T>;
    type->release = cpp_importing::templated_destroy<T>;
    type->assign = cpp_importing::templated_assign<T>;
    type->cppTypeInfo = &typeid(T);
}

template <class T>
void import_pointer_type(Term* term)
{
    Type* type = &as_type(term);
    reset_type(type);

    type->cppTypeInfo = &typeid(T);
}

template <class T>
Term* import_pointer_type(Branch& branch, std::string name="")
{
    Term* term = create_type(branch, name);
    import_pointer_type<T>(term);
    return term;
}

} // namespace circa

#endif
