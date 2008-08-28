
#include "bootstrapping.h"
#include "term.h"

namespace circa {
namespace alien {

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
void templated_copy(Term* source, Term* dest)
{
    *(reinterpret_cast<T*>(dest->value)) = *(reinterpret_cast<T*>(source->value));
}

template <class T>
Term* quick_create_type_templated(Branch* branch, std::string name)
{
    return quick_create_type(branch, name,
        templated_alloc<T>,
        templated_dealloc<T>,
        templated_copy<T>,
        NULL);
}

} // namespace alien
} // namespace circa
