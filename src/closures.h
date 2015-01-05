// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void insert_nonlocal_terms(Block* block);

void closure_block_evaluate(Stack* stack);

void closure_save_bindings_for_frame(Value* closure, Frame* frame);

int count_closure_upvalues(Block* block);

// Returns 0 if no closure bindings are found. Caller should use count_closure_upvalues
// to determine if there really are any bindings.
int find_first_closure_upvalue(Block* block);

void closure_save_all_bindings(Value* closure, Stack* stack);

bool is_closure(Value* value);

// 'bindings' can be NULL.
void set_closure(Value* value, Block* block, Value* bindings);

Block* func_block(Value* value);
Value* func_bindings(Value* value);
Value* closure_get_block(Value* value);
Value* closure_get_bindings(Value* value);

void closures_install_functions(NativePatch* patch);

} // namespace circa
