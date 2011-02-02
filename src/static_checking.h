// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "types/list.h"

namespace circa {

struct StaticErrorCheck
{
    List errors;

    // Structure of each item in errors:
    //  [0] int index
    //  [1] string type
    //  [2] int inputIndex (only used for errors related to inputs)

    int count() { return errors.length(); }
    bool empty() { return count() == 0; }
    std::string toString() { return errors.toString(); }
};

void check_for_static_errors(StaticErrorCheck* result, Branch& branch);

void print_static_errors_formatted(StaticErrorCheck* result, std::ostream& out);

// Convenience methods, where the caller doesn't need to create a StaticErrorCheck object:
bool has_static_error(Term* term);
bool has_static_errors(Branch& branch);
int count_static_errors(Branch& branch);

void print_static_errors_formatted(Branch& branch, std::ostream& out);
void print_static_error(Term* term, std::ostream& out);

std::string get_static_errors_formatted(Branch& branch);
std::string get_static_error_message(Term* term);

} // namespace circ
