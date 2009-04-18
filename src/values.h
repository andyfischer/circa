// Copyright 2008 Paul Hodge

#ifndef CIRCA_VALUES_INCLUDED
#define CIRCA_VALUES_INCLUDED

// values.h
//
// Functions for dealing with values of terms, including allocating and deallocating.

#include "common_headers.h"

namespace circa {

void alloc_value(Term* term);
void dealloc_value(Term* term);

void assign_value(Term* source, Term* dest);
void assign_value_but_dont_copy_inner_branch(Term* source, Term* dest);

bool is_value_alloced(Term* term);

} // namespace circa

#endif
