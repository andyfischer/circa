// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

bool is_function_stateful(Term* func);

Term* find_active_state_container(Block* block);
Term* find_or_create_default_state_input(Block* block);

// If the block has state, this adds a pack_state call that captures state
// values at the current position. If the block has no state then this
// returns NULL.
Term* block_add_pack_state(Block* block);

bool is_declared_state(Term* term);

// Update the block's stateType. Should be called after the code is changed in a way
// that could add/remove declared state.
void block_update_state_type(Block* block);
bool block_has_inline_state(Block* block);
void block_mark_state_type_dirty(Block* block);

void block_update_pack_state_calls(Block* block);

// Runtime calls.
void unpack_state(caStack* stack);
void pack_state(caStack* stack);

void declared_state_format_source(caValue* source, Term* term);

} // namespace circa
