// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Check if 'possibleAccessor' is an accessor expresion that we understand. If so, then create
// a set_with_selector expression which rebinds the root name to use 'result' instead.
// If 'possibleAccessor' isn't an accessor, then just create a named copy from 'result'.
Term* rebind_possible_accessor(Block* block, Term* possibleAccessor, Term* result);

bool term_is_accessor_traceable(Term* accessor);
Term* resolve_rebind_operators_in_inputs(Block* block, Term* result);

Value* get_with_selector(Value* root, Value* selector, Value* error);
void set_with_selector(Value* root, Value* selector, Value* newValue, Value* error);

// New replacement for selector: the "path" object. Only works on hashtable values.
Value* path_touch_and_init_map(Value* value, Value* path);
Value* path_get(Value* value, Value* path);
void path_delete(Value* value, Value* path);

void selector_setup_funcs(NativePatch* patch);

} // namespace circa
