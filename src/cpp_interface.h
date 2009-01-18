// Copyright 2008 Andrew Fischer

#ifndef CIRCA_CPP_INTERFACE_INCLUDED
#define CIRCA_CPP_INTERFACE_INCLUDED

#include "importing.h"
#include "parser.h"
#include "term.h"
#include "type.h"

namespace circa {
namespace cpp_interface {

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
void templated_duplicate(Term* source, Term* dest)
{
    dest->value = new T(*(reinterpret_cast<T*>(source->value)));
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

template <class T>
std::string templated_toString(Term* term)
{
    return reinterpret_cast<T*>(term->value)->toString();
}

} // namespace cpp_interface

// Public functions

template <class T>
Term* import_type(Branch& branch, std::string name="")
{
    Term* term = quick_create_type(branch, name);
    Type& type = as_type(term);
    type.alloc = cpp_interface::templated_alloc<T>;
    type.dealloc = cpp_interface::templated_dealloc<T>;
    type.duplicate = cpp_interface::templated_duplicate<T>;
    type.assign = cpp_interface::templated_assign<T>;
    type.cppTypeInfo = &typeid(T);
    return term;
}

template <class T>
void register_cpp_toString(Term* type)
{
    as_type(type).toString = cpp_interface::templated_toString<T>;
}

template <class T>
T& as(Term* term)
{
    Type& type = as_type(term->type);

    if (type.cppTypeInfo == NULL)
        throw std::runtime_error(std::string("type ") + type.name
                + " is not a C++ type");

    if (*type.cppTypeInfo != typeid(T)) {
        std::stringstream error;
        error << "C++ type mismatch, existing data has type " << type.name;
        throw std::runtime_error(error.str());
    }

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
