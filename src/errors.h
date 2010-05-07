// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

namespace circa {

// Puts 'errorTerm' into an error state, with the given message.
void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message);

// Signal that an unexpected Circa error has occurred. Depending on debug settings, these
// will either throw an exception or trigger an assert.
void internal_error(const char* message);

// Trigger internal_error() with a string that describes the current line of code
#define circa_assert(x) circa_assert_unmacro((x), #x, __LINE__, __FILE__)
void circa_assert_unmacro(bool expr, const char* exprStr, int line, const char* file);

// Signal that a type mismatch has occurred in native code.
void native_type_mismatch(std::string const& message);

// Check if term has the given type, calls native_type_mismatch if not.
void assert_type(Term* term, Term* type);

bool has_static_error(Term* term);
bool has_static_errors(Branch& branch);

std::string get_static_errors_formatted(Branch& branch);
std::string get_static_error_message(Term* term);

void print_static_error_formatted(Term* term, std::ostream& output);
void print_static_errors_formatted(Branch& branch, std::ostream& output);

void print_runtime_error_formatted(EvalContext& context, std::ostream& output);

enum StaticError {
    SERROR_NO_ERROR = 0,
    SERROR_NULL_FUNCTION,
    SERROR_WRONG_NUMBER_OF_INPUTS,
    SERROR_NULL_INPUT_TERM,
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
