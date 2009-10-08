// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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
    term->value = new T();
}

template <class T>
void templated_dealloc(Term* type, Term* term)
{
    delete reinterpret_cast<T*>(term->value);
}

template <class T>
void templated_assign(Term* source, Term* dest)
{
    *reinterpret_cast<T*>(dest->value) = *reinterpret_cast<T*>(source->value);
}

template <class T>
bool templated_equals(Term* a, Term* b)
{
    return (*(reinterpret_cast<T*>(a->value))) == (*(reinterpret_cast<T*>(b->value)));
}

template <class T>
bool templated_lessThan(Term* a, Term* b)
{
    return (*(reinterpret_cast<T*>(a->value))) < (*(reinterpret_cast<T*>(b->value)));
}

void pointer_alloc(Term* type, Term* term);
bool raw_value_equals(Term* a, Term* b);
bool raw_value_less_than(Term* a, Term* b);

} // namespace cpp_importing

// Public functions

template <class T>
void import_type(Term* term)
{
    type_t::get_alloc_func(term) = cpp_importing::templated_alloc<T>;
    type_t::get_dealloc_func(term) = cpp_importing::templated_dealloc<T>;
    type_t::get_assign_func(term) = cpp_importing::templated_assign<T>;
    type_t::get_std_type_info(term) = &typeid(T);
    type_t::get_to_string_func(term) = NULL;
    type_t::get_remap_pointers_func(term) = NULL;
    type_t::get_check_invariants_func(term) = NULL;
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
