
#include "bootstrapping.h"
#include "term.h"

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

} // namespace cpp_interface

// Public functions

template <class T>
Term* quick_create_cpp_type(Branch* branch, std::string name)
{
    return quick_create_type(branch, name,
        cpp_interface::templated_alloc<T>,
        cpp_interface::templated_dealloc<T>,
        cpp_interface::templated_duplicate<T>,
        NULL);
}

template <class T>
T& as(Term* term)
{
    return *dynamic_cast<T*>(term->value);
}

} // namespace circa
