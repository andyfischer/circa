// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Write a set_with_selector expression that creates a selector() from 'accessor', and
// assigns 'result' to the subelement. If the accessor has no selector then we simply
// rename 'result' to have 'accessor's name.

// Check if 'possibleAccessor' is an accessor expresion that we understand. If so, then create
// a set_with_selector expression which rebinds the root name to use 'result' instead.
// If 'possibleAccessor' isn't an accessor, then just create a named copy from 'result'.
Term* rebind_possible_accessor(Block* block, Term* possibleAccessor, Term* result);

bool term_is_accessor_traceable(Term* accessor);
void resolve_rebind_operators_in_inputs(Block* block, Term* result);

caValue* get_with_selector(caValue* root, caValue* selector, caValue* error);
void set_with_selector(caValue* root, caValue* selector, caValue* newValue, caValue* error);

void selector_setup_funcs(Block* kernel);

} // namespace circa
