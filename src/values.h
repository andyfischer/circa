// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_VALUES_INCLUDED
#define CIRCA_VALUES_INCLUDED

#include "common_headers.h"

namespace circa {

#if 0
bool share_value(Term* original, Term* share);
#endif

bool is_value_alloced(Term* term);
void alloc_value(Term* term);
void dealloc_value(Term* term);

} // namespace circa

#endif
