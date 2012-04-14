// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "list.h"

namespace circa {

void check_for_static_errors(List* result, Branch* branch);
void update_static_error_list(Branch* branch);

void format_static_error(caValue* error, caValue* stringOutput);

// Print each static error to 'out'. Returns true if there were any static errors.
bool print_static_errors_formatted(List* result, std::ostream& out);


bool has_static_error(Term* term);
bool has_static_errors(Branch* branch);
bool has_static_errors_cached(Branch* branch);
int count_static_errors(Branch* branch);

bool print_static_errors_formatted(Branch* branch, std::ostream& out);
bool print_static_errors_formatted(Branch* branch);
bool print_static_errors_formatted(Branch* branch, caValue* out);

void print_static_error(Term* term, std::ostream& out);
std::string get_static_errors_formatted(Branch* branch);
std::string get_static_error_message(Term* term);

void mark_static_error(Term* term, const char* msg);
void mark_static_error(Term* term, caValue* error);

} // namespace circ
