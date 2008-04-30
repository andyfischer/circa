#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#include "CommonHeaders.h"
#include "Term.h"

namespace type {

struct Type
{
   Type() :
      initialize_data(NULL)
   {}
      
   // Code
   void (*initialize_data)(Term*);

};


void call_initialize_data(Term* type, Term* target);
void set_initialize_data_func(Term* target, void(*func)(Term*));
void initialize_data_for_types(Term* type);

}


#endif
