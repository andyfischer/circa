
#include "CommonHeaders.h"

#include "Term.h"
#include "Type.h"

namespace type {

#define TYPE(t) (reinterpret_cast<Type*>((t)->data))

void call_initialize_data(Term* type, Term* target)
{
   TYPE(type)->initialize_data(target);
}

void set_initialize_data_func(Term* target, void(*func)(Term*))
{
   TYPE(target)->initialize_data = func;
}

void initialize_data_for_types(Term* term)
{
   assert(term->data == NULL);
   term->data = new Type;
}

} // namespace type
