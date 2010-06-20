// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// Various code for debugging purposes. This code shouldn't have an effect on a
// release build.

#pragma once

#include "common_headers.h"

namespace circa {

// This flag helps identify pointers to deleted Terms. We keep a 
// global set of 'good' pointers, and any call to assert_valid_term
// will check that the given piece of memory is valid. This option
// comes with a performance penalty.
#define DEBUG_CHECK_VALID_TERM_POINTERS 0

// Checks if term is a valid pointer according to our map, and triggers
// an assert if not. Only has an effect if DEBUG_CHECK_VALID_TERM_POINTERS
// is enabled.
void assert_valid_term(Term* term);

// Returns whether this pointer is valid according to our map. Only has
// an effect if DEBUG_CHECK_VALID_TERM_POINTERS is enabled. It's recommended
// that you use assert_valid_term instead of this function, since this one
// is easier to abuse.
bool debug_is_term_pointer_valid(Term* term);

// Setting this to true will make us abort trap on the next name lookup.
extern bool DEBUG_TRAP_NAME_LOOKUP;

// Setting this to true will make us abort trap on the next call to error_occurred()
extern bool DEBUG_TRAP_ERROR_OCCURRED;

// Spit out this branch's raw contents to std::cout
void dump_branch(Branch& branch);
void dump_branch_with_props(Branch& branch);

} // namespace circa
