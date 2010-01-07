// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_ERRORS_INCLUDED
#define CIRCA_ERRORS_INCLUDED

namespace circa {

// Puts 'errorTerm' into an error state, with the given message.
void error_occurred(Term* errorTerm, std::string const& message);

// Signal that a type mismatch has occurred in native code.
void native_type_mismatch(std::string const& message);

// Check if term has the given type, calls native_type_mismatch if not.
void assert_type(Term* term, Term* type);

// Puts 'errorTerm' into an error state, with no message. This term should contain a 
// branch, and somewhere inside that branch there should be an errored term with
// a message.
void nested_error_occurred(Term* errorTerm);

// Remove an error status.
void clear_error(Term* term);

// Check if this term is marked with a runtime error.
bool has_runtime_error(Term* term);

bool has_runtime_error_message(Term* term);
std::string get_runtime_error_message(Term* term);

// Check if there is an errorred term inside this branch.
bool has_runtime_error(Branch& branch);

bool has_static_error(Term* term);
bool has_static_errors(Branch& branch);

std::string get_runtime_error_formatted(Branch& branch);
std::string get_static_errors_formatted(Branch& branch);
std::string get_static_error_message(Term* term);
std::string get_error_message(Term* term);

void print_runtime_error_formatted(Term* term, std::ostream& output);
void print_runtime_error_formatted(Branch& branch, std::ostream& output);
void print_static_error_formatted(Term* term, std::ostream& output);
void print_static_errors_formatted(Branch& branch, std::ostream& output);

bool has_error(Term* term);
bool has_error(Branch& branch);

enum StaticError {
    SERROR_NO_ERROR = 0,
    SERROR_NULL_FUNCTION,
    SERROR_WRONG_NUMBER_OF_INPUTS,
    SERROR_NULL_INPUT_TERM,
    SERROR_INPUT_HAS_ERROR,
    SERROR_INPUT_TYPE_ERROR,
    SERROR_UNKNOWN_FUNCTION,
    SERROR_UNKNOWN_TYPE,
    SERROR_UNKNOWN_IDENTIFIER,
    SERROR_UNKNOWN_FIELD,
    SERROR_UNRECGONIZED_EXPRESSION
};

StaticError get_static_error(Term* term);
int count_static_errors(Branch& branch);

} // namespace circa

#endif
