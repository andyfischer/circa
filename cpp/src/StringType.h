#ifndef STRING_TYPE_H_INCLUDED
#define STRING_TYPE_H_INCLUDED

#include "CommonHeaders.h"

class Term;

namespace string_type {

void initialize_data(Term* term);
void set_data(Term* term, string value);
std::string to_string(Term* term);

}

#endif
