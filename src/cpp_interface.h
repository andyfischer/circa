// Copyright 2008 Andrew Fischer

#ifndef CIRCA_CPP_INTERFACE_INCLUDED
#define CIRCA_CPP_INTERFACE_INCLUDED

#include "bootstrapping.h"
#include "term.h"
#include "type.h"

namespace circa {
namespace cpp_interface {

template <class T>
void templated_alloc(Term* term)
{
    term->value = new T();
}

template <class T>
void templated_dealloc(Term* term)
{
    delete reinterpret_cast<T*>(term->value);
    term->value = NULL;
}

template <class T>
void templated_duplicate(Term* source, Term* dest)
{
    dest->value = new T(*(reinterpret_cast<T*>(source->value)));
}

template <class T>
bool templated_equals(Term* a, Term* b)
{
    return (*(reinterpret_cast<T*>(a->value))) == (*(reinterpret_cast<T*>(b->value)));
}

template <class T>
std::string templated_toString(Term* term)
{
    return reinterpret_cast<T*>(term->value)->toString();
}

} // namespace cpp_interface

template <class CppType>
void assign_from_cpp_type(Type& type)
{
    type.alloc = cpp_interface::templated_alloc<CppType>;
    type.dealloc = cpp_interface::templated_dealloc<CppType>;
    type.duplicate = cpp_interface::templated_duplicate<CppType>;
    //type.equals = cpp_interface::templated_equals<CppType>;
}

// Public functions

template <class T>
Term* quick_create_cpp_type(Branch* branch, std::string name)
{
    Term* term = quick_create_type(branch, name);
    assign_from_cpp_type<T>(*as_type(term));
    return term;
}

template <class T>
T& as(Term* term)
{
    // todo: add type checking
    return *((T*) term->value);
}

} // namespace circa

#endif
