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

Term* import_value(Branch* branch, Term* type, void* initialValue, std::string const& name="");
Term* import_value(Branch* branch, std::string const& typeName, void* initialValue, std::string const& name="");

Term* string_value(Branch* branch, std::string const& s, std::string const& name="");
Term* int_value(Branch* branch, int i, std::string const& name="");
Term* float_value(Branch* branch, float f, std::string const& name="");
Term* bool_value(Branch* branch, bool b, std::string const& name="");

} // namespace circa

#endif
