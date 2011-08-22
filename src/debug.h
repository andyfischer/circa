// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// Various code for debugging purposes. This code shouldn't have an effect on a
// release build.

#pragma once

#include "common_headers.h"

namespace circa {

// Setting this to true will make us abort trap on the next name lookup.
extern bool DEBUG_TRAP_NAME_LOOKUP;

// Setting this to true will make us abort trap on the next call to error_occurred()
extern bool DEBUG_TRAP_ERROR_OCCURRED;

// Setting this to true will write to stdout for every write to a Ref value.
extern bool DEBUG_TRACE_ALL_REF_WRITES;

extern bool DEBUG_TRACE_ALL_TERM_DESTRUCTORS;

// Spit out this branch's raw contents to std::cout
void dump(Branch& branch);
void dump_with_props(Branch& branch);

void dump(TaggedValue& value);
void dump(TaggedValue* value);

// Signal that an unexpected error has occurred. Depending on debug settings, this
// will either throw an exception or trigger an assert().
void internal_error(const char* message);
void internal_error(std::string const& message);

} // namespace circa
