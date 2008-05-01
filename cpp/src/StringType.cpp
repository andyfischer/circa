
#include "StringType.h"
#include "Term.h"

namespace string_type {

#define STR(t) (*(reinterpret_cast<string*>((t)->data)))

void initialize_data(Term* term)
{
   assert(term->data == NULL);
   term->data = new string;
}

void set_data(Term* term, string value)
{
   STR(term) = value;
}

std::string to_string(Term* term)
{
   return STR(term);
}

}
