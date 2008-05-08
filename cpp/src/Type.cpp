
#include "CommonHeaders.h"

#include "Term.h"
#include "Type.h"

namespace type {

#define TYPE(term) (reinterpret_cast<Type*>((term)->data))

void call_initialize_data(Term* type, Term* target)
{
   Type* type_data = TYPE(type);

   if (type_data->initialize_data == NULL) {
      printf("ERROR: type %s is incomplete (no initialize_data).\n",
            type_data->name.c_str());
      return;
   }

   type_data->initialize_data(target);
}

string call_to_string(Term* type, Term* target)
{
   Type* type_data = TYPE(type);
   if (type_data->to_string == NULL) {
      printf("ERROR: type %s is incomplete (no to_string).\n",
            type_data->name.c_str());
      return "";
   }

   return type_data->to_string((Term*) target);
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
    std::stringstream sstream;
    sstream << "<Type " << TYPE(type)->name << ">";
    return sstream.str();
}

void set_name(Term* type, const string& name)
{
    TYPE(type)->name = name;
}

} // namespace type
