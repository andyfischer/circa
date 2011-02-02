// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Puts 'errorTerm' into an error state, with the given message.
void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message);

// Signal that an unexpected error has occurred. Depending on debug settings, this
// will either throw an exception or trigger an assert().
void internal_error(const char* message);
void internal_error(std::string const& message);

void print_runtime_error_formatted(EvalContext& context, std::ostream& output);

} // namespace circa
