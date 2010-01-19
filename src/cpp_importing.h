// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_CPP_INTERFACE_INCLUDED
#define CIRCA_CPP_INTERFACE_INCLUDED

#include "importing.h"
#include "term.h"
#include "type.h"

namespace circa {
namespace cpp_importing {

template <class T>
void templated_alloc(Term* type, Term* term)
{
    // Use malloc() and memset(), this allows us to make sure that the value's memory
    // is zeroed out before it is used.
    void* data = malloc(sizeof(T));
    memset(data, 0, sizeof(T));
    new(data) T();
    term->value.type = &as_type(type);
    term->value.data.ptr = data;
}

template <class T>
void templated_dealloc(Term* type, Term* term)
{
    // Placement delete because we used placement new above.
    reinterpret_cast<T*>(term->value.data.ptr)->~T();
    free((void*) term->value.data.ptr);
    term->value.data.ptr = 0;
    term->value.type = 0;
}

template <class T>
void templated_assign(Term* source, Term* dest)
{
    *reinterpret_cast<T*>(dest->value.data.ptr)
        = *reinterpret_cast<T*>(source->value.data.ptr);
}

template <class T>
bool templated_equals(Term* a, Term* b)
{
    return (*(reinterpret_cast<T*>(a->value.data.ptr)))
        == (*(reinterpret_cast<T*>(b->value.data.ptr)));
}

template <class T>
bool templated_lessThan(Term* a, Term* b)
{
    return (*(reinterpret_cast<T*>(a->value.data.ptr)))
        < (*(reinterpret_cast<T*>(b->value.data.ptr)));
}

void pointer_alloc(Term* type, Term* term);
bool raw_value_less_than(Term* a, Term* b);

} // namespace cpp_importing

// Public functions

template <class T>
void import_type(Term* term)
{
    Type* type = &as_type(term);

    type->alloc = cpp_importing::templated_alloc<T>;
    type->dealloc = cpp_importing::templated_dealloc<T>;
    type->assign = cpp_importing::templated_assign<T>;
    type->cppTypeInfo = &typeid(T);
    type->toString = NULL;
    type->remapPointers = NULL;
    type->checkInvariants = NULL;
}

template <class T>
Term* import_type(Branch& branch, std::string name="")
{
    Term* term = create_type(branch, name);
    import_type<T>(term);
    return term;
}

template <class T>
void import_pointer_type(Term* term)
{
    type_t::get_alloc_func(term) = cpp_importing::pointer_alloc;
    type_t::get_assign_func(term) = shallow_assign;
    type_t::get_equals_func(term) = shallow_equals;
    type_t::get_std_type_info(term) = &typeid(T);
    type_t::get_to_string_func(term) = NULL;
    type_t::get_remap_pointers_func(term) = NULL;
    type_t::get_check_invariants_func(term) = NULL;
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
