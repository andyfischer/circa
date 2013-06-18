// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "block.h"
#include "object.h"
#include "term.h"

namespace circa {

// Create a declared function
Term* create_function(Block* block, const char* name);

void finish_building_function(Block* func);

Type* derive_specialized_output_type(Term* function, Term* call);

// Returns whether this term rebinds the input at 'index'
bool function_call_rebinds_input(Term* term, int index);

void function_format_header_source(caValue* source, Block* func);
void function_format_source(caValue* source, Term* term);

bool is_subroutine(Term* term);
bool is_subroutine(Block* block);

} // namespace circa
