// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// Various code for debugging purposes. This code shouldn't have an effect on a
// release build.

#pragma once

#include "common_headers.h"

namespace circa {

// Setting this to true will make us abort trap on the next name lookup.
extern bool DEBUG_TRAP_NAME_LOOKUP;

// Setting this to true will make us abort trap on the next call to error_occurred()
extern bool DEBUG_TRAP_ERROR_OCCURRED;

// Spit out this branch's raw contents to std::cout
void dump_branch(Branch& branch);
void dump_bytecode(Branch& branch);
void dump_branch_with_props(Branch& branch);

} // namespace circa
