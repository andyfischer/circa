// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// update_cascades.cpp
//
// This module deals with updates to Terms and Blockes which cause changes
// in other parts of the code. For example, changing an input argument to a
// function call might change its output type.

namespace circa {

// Called when the staticErrors list on the Block should be recalculated.
void mark_static_errors_invalid(Block* block);

void finish_update_cascade(Block* block);
void recursively_finish_update_cascade(Block* block);

void on_block_inputs_changed(Block* block);

void fix_forward_function_references(Block* block);

void dirty_bytecode(Block* block);
void refresh_bytecode(Block* block);

} // namespace circa
