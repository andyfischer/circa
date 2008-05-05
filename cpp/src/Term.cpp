
#include "Function.h"
#include "Term.h"
#include "Type.h"


Term* Term::get_type() const
{
   if (this->function == NULL) {
      printf("ERROR: In get_type, term %s has NULL function.\n",
        this->debug_name.c_str());
      return NULL;
   }

   return function::output_type(this->function);
}

string Term::to_string() const
{
   if (this == NULL) {
      printf("ERROR: Term::to_string called on null pointer.\n");
      return "";
   }

   if (this->get_type() == NULL) {
      printf("ERROR: %s has a null type.\n", this->debug_name.c_str());
      return "";
   }

   return type::call_to_string(this->get_type(), (Term*) this);
}
