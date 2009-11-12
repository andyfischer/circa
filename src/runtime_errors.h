// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

namespace circa {

// Puts 'errorTerm' into an error state, with the given message.
void error_occurred(Term* errorTerm, std::string const& message);

// Remove an error status.
void clear_error(Term* term);

// Puts 'errorTerm' into an error state, with no message. This term should contain a 
// branch, and somewhere inside that branch there should be an errored term with
// a message.
void nested_error_occurred(Term* errorTerm);

// Check if this term is marked with a runtime error.
bool has_runtime_error(Term* term);

// Check if there is an errorred term inside this branch.
bool has_runtime_error(Branch& branch);

std::string get_runtime_error_message(Term* term);

void print_runtime_error_formatted(Branch& branch, std::ostream& output);
std::string get_runtime_error_formatted(Branch& branch);

// Signal that a type mismatch has occurred in native code.
void native_type_mismatch(std::string const& message);

// Check if term has the given type, calls native_type_mismatch if not.
void assert_type(Term* term, Term* type);

// Checks for either a static error or runtime error.
bool has_error(Term* term);

// Return either a static or runtime error message.
std::string get_error_message(Term* term);

}
