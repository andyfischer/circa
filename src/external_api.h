// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_EXTERNAL_API_INCLUDED
#define CIRCA_EXTERNAL_API_INCLUDED

#include "common_headers.h"

namespace circa {

int int_input(Term* term, int index);
float float_input(Term* term, int index);
bool bool_input(Term* term, int index);
const char* string_input(Term* term, int index);

} // namespace circa

#endif
