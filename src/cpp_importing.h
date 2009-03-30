// Copyright 2008 Andrew Fischer

#ifndef CIRCA_CPP_INTERFACE_INCLUDED
#define CIRCA_CPP_INTERFACE_INCLUDED

#include "importing.h"
#include "term.h"
#include "type.h"

namespace circa {
namespace cpp_importing {

template <class T>
void* templated_alloc(Term* typeTerm)
{
    return new T();
}

template <class T>
void templated_dealloc(void* data)
{
    delete reinterpret_cast<T*>(data);
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

} // namespace cpp_importing

// Public functions

template <class T>
Term* import_type(Branch& branch, std::string name="")
{
    Term* term = quick_create_type(branch, name);
    Type& type = as_type(term);
    type.alloc = cpp_importing::templated_alloc<T>;
    type.dealloc = cpp_importing::templated_dealloc<T>;
    type.assign = cpp_importing::templated_assign<T>;
    type.cppTypeInfo = &typeid(T);
    return term;
}

template <class T>
T& as(Term* term)
{
    Type& type = as_type(term->type);

    if (type.cppTypeInfo == NULL)
        throw std::runtime_error("type "+type.name+" is not a C++ type");

    if (*type.cppTypeInfo != typeid(T))
        throw std::runtime_error("C++ type mismatch, existing data has type "+type.name);

    return *((T*) term->value);
}

template <class T>
T eval_as(std::string const& statement)
{
    Branch branch;

    Term* result_term = branch.eval(statement);

    if (result_term->hasError())
        throw std::runtime_error(result_term->getErrorMessage());

    return as<T>(result_term);
}

template <class T>
T eval_as(Branch &branch, std::string const& statement)
{
    Term* result = branch.eval(statement);

    if (result->hasError())
        throw std::runtime_error(result->getErrorMessage());

    return as<T>(result);
}

} // namespace circa

#endif
