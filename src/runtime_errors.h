// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

namespace circa {

// Puts 'errorTerm' into an error state, with the given message.
void error_occurred(Term* errorTerm, std::string const& message);

// Puts 'errorTerm' into an error state, with no message. This term should contain a 
// branch, and somewhere inside that branch there should be an errored term with
// a message.
void nested_error_occurred(Term* errorTerm);

void print_runtime_error_formatted(Branch& branch, std::ostream& output);

std::string get_runtime_error_formatted(Branch& branch);

}
