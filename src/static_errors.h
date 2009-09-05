// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_STATIC_ERRORS_INCLUDED
#define CIRCA_STATIC_ERRORS_INCLUDED

// Code for checking and displaying static errors. A static error is any problem with
// the code that we can find without running it. This includes parser errors and type
// errors.

#include "common_headers.h"

namespace circa {

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

bool has_static_error(Term* term);
StaticError get_static_error(Term* term);
std::string get_static_error_message(Term* term);
int count_static_errors(Branch& branch);
bool has_static_errors(Branch& branch);
void print_static_errors_formatted(Branch& branch, std::ostream& output);
std::string get_static_errors_formatted(Branch& branch);

} // namespace circa

#endif
