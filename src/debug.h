// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

// Various code for debugging purposes. This code shouldn't have an effect on a
// release build.

#ifndef CIRCA_DEBUG_INCLUDED
#define CIRCA_DEBUG_INCLUDED

#include "common_headers.h"

namespace circa {

// This flag helps identify pointers to deleted Terms. We keep a 
// global set of 'good' pointers, and any call to assert_good_pointer
// will check that the given piece of memory is valid. This option
// comes with a performance penalty.
//
// It's recommended that you don't write code that calls is_bad_pointer,
// because this would be easy to abuse. Instead, use assert_good_pointer.
#define DEBUG_CHECK_FOR_BAD_POINTERS 0

// Enabling this flag causes us to never actually delete Term objects.
// This removes the possibility that a bad pointer will mistakenly be
// deemed a good pointer, just because the new Term occupies the same
// memory that a previous Term did. The drawback to this option is
// of course, unbounded memory consumption.
#define DEBUG_NEVER_DELETE_TERMS 0

// Setting this to true will make us abort trap on the next name lookup.
extern bool DEBUG_TRAP_NAME_LOOKUP;

// Setting this to true will make us abort trap on the next call to error_occurred()
extern bool DEBUG_TRAP_ERROR_OCCURRED;

void register_good_pointer(Term* term);
void unregister_good_pointer(Term* term);
void assert_good_pointer(Term* term);
bool is_bad_pointer(Term* term);


// Spit out this branch's raw contents to std::cout
void dump_branch(Branch& branch);
void dump_branch_with_props(Branch& branch);

} // namespace circa

#endif
