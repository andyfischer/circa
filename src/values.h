// Copyright 2008 Andrew Fischer

#ifndef CIRCA_VALUES_INCLUDED
#define CIRCA_VALUES_INCLUDED

#include "common_headers.h"

namespace circa {

// recycle_value will either call duplicate_value or steal_value, depending
// on heuristics
void recycle_value(Term* source, Term* dest);

void duplicate_value(Term* source, Term* dest);

// Attempt to 'steal' the output value from source. This is more efficient
// than copying, and useful if 1) dest needs a copy of source's value, and
// 2) you don't think that anyone will need source.
//
// Note that (2) is not a hard requirement since we can usually recreate the
// value. But it may be less efficient to do so.
//
// In some situations we are not allowed to steal a value. In these situations,
// calling steal_value is equivalent to calling duplicate_value. These situations include:
//  1) if source is a constant
//  2) ... (possible future situations)
void steal_value(Term* source, Term* dest);

void dealloc_value(Term* term);

Term* string_var(Branch& branch, std::string const& s, std::string const& name="");
Term* int_var(Branch& branch, int i, std::string const& name="");
Term* float_var(Branch& branch, float f, std::string const& name="");
Term* list_var(Branch& branch, ReferenceList list, std::string const& name="");

} // namespace circa

#endif
