#ifndef INT_TYPE_H_INCLUDED
#define INT_TYPE_H_INCLUDED

#include "CommonHeaders.h"

class Term;

namespace int_type {

void initialize_data(Term* term);
void set_data(Term* term, int value);
string to_string(Term* term);

}

#endif
