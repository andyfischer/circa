// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// Various code for debugging purposes. This code shouldn't have an effect on a
// release build.

#pragma once

#include "common_headers.h"

namespace circa {

// Setting this to true will make us abort trap on the next name lookup.
extern bool DEBUG_TRAP_NAME_LOOKUP;

// Setting this to true will make us abort trap on the next call to raise_error()
extern bool DEBUG_TRAP_RAISE_ERROR;

// Setting this to true will write to stdout for every write to a Ref value.
extern bool DEBUG_TRACE_ALL_REF_WRITES;

extern bool DEBUG_TRACE_ALL_TERM_DESTRUCTORS;

extern const char* DEBUG_BREAK_ON_TERM;

// Spit out this branch's raw contents to std::cout
void dump(Branch& branch);
void dump(Branch* branch);
void dump_with_props(Branch& branch);
void dump(Term* term);
void dump(TaggedValue& value);
void dump(TaggedValue* value);
void dump(EvalContext* context);

// Signal that an unexpected error has occurred. Depending on debug settings, this
// will either throw an exception or trigger an assert().
void internal_error(const char* message);
void internal_error(std::string const& message);

void ca_debugger_break();

} // namespace circa
