#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#include "CommonHeaders.h"
#include "Term.h"

namespace type {

struct Type
{
   Type() :
      name("undefined_Type"),
      initialize_data(NULL)
   {}

   string name;
      
   // Code
   void (*initialize_data)(Term*);
   string (*to_string)(Term*);

};


void call_initialize_data(Term* type, Term* target);
void set_initialize_data_func(Term* target, void(*func)(Term*));
void set_to_string_func(Term* target, string(*func)(Term*));
void initialize_data_for_types(Term* type);

}


#endif
