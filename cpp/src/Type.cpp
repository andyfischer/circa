
#include "CommonHeaders.h"

#include "Term.h"
#include "Type.h"

namespace type {

#define TYPE(t) (reinterpret_cast<Type*>((t)->data))

void call_initialize_data(Term* type, Term* target)
{
   assert(TYPE(type)->initialize_data != NULL);
   TYPE(type)->initialize_data(target);
}

void set_initialize_data_func(Term* target, void(*func)(Term*))
{
   TYPE(target)->initialize_data = func;
}

void set_to_string_func(Term* target, string(*func)(Term*))
{
   TYPE(target)->to_string = func;
}

void initialize_data_for_types(Term* term)
{
   assert(term->data == NULL);
   term->data = new Type;
}

std::string to_string(Term* type)
{
   return TYPE(type)->name;
}

} // namespace type
