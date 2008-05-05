
#include "CommonHeaders.h"

#include "Term.h"
#include "Type.h"

namespace type {

#define TYPE(term) (reinterpret_cast<Type*>((term)->data))

void call_initialize_data(Term* type, Term* target)
{
   Type* t = TYPE(type);

   if (t->initialize_data == NULL) {
      printf("ERROR: type %s is incomplete (no initialize_data).\n",
            t->name.c_str());
      return;
   }
   t->initialize_data(target);
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
   term->data = new Type;
}

string to_string(Term* type)
{
   return TYPE(type)->name;
}

} // namespace type
