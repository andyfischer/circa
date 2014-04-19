// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "list.h"

namespace circa {

void check_for_static_errors(Value* result, Block* block);
void update_static_error_list(Block* block);

void format_static_error(caValue* error, caValue* out);
void format_static_error(Term* term, caValue* out);
void format_static_errors(caValue* errorList, caValue* out);

// Print each static error to 'out'. Returns true if there were any static errors.
bool print_static_errors_formatted(Block* block, caValue* out);
bool print_static_errors_formatted(caValue* result, caValue* out);

bool has_static_error(Term* term);
bool has_static_errors(Block* block);
bool has_static_errors_cached(Block* block);
int count_static_errors(Block* block);

bool print_static_errors_formatted(Block* block);
bool print_static_errors_formatted(Block* block, caValue* out);

#if 0
std::string get_static_errors_formatted(Block* block);
std::string get_static_error_message(Term* term);
#endif

void mark_static_error(Term* term, const char* msg);
void mark_static_error(Term* term, caValue* error);

} // namespace circ
