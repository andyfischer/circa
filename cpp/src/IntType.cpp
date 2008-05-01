
#include "CommonHeaders.h"

#include "IntType.h"
#include "Term.h"

namespace int_type {

#define INT(term) (reinterpret_cast<int>((term)->data))

void initialize_data(Term* term)
{
   // do nothing
   assert(term->data == NULL);
}

void set_data(Term* term, int value)
{
   term->data = reinterpret_cast<void*>(value);
}

string to_string(Term* term)
{
   std::stringstream sstream;
   sstream << INT(term);
   return sstream.str();
}

}
