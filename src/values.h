// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_VALUES_INCLUDED
#define CIRCA_VALUES_INCLUDED

#include "common_headers.h"

namespace circa {

bool is_value_alloced(Term* term);
void alloc_value(Term* term); // deprecated
void dealloc_value(Term* term); // deprecated
void assign_value(Term* source, Term* dest);

} // namespace circa

#endif
