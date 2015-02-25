// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "block.h"
#include "term.h"

namespace circa {

// Create a declared function
Term* create_function(Block* block, const char* name);

void update_static_closure_force(Term* term);
void update_static_closure_if_possible(Term* term);
void finish_building_function(Block* func);

Type* derive_specialized_output_type(Term* function, Term* call);

} // namespace circa
