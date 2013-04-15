// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Look through every term in 'target', and see if it contains a reference to a term in
// 'oldBlock'. If so, migrate that reference to the equivalent term (according to the
// relative unique names) inside 'newBlock'. If the equivalent in 'newBlock' isn't found,
// then the reference will be set to null.
void update_all_code_references(Block* target, Block* oldBlock, Block* newBlock);

Term* translate_term_across_blocks(Term* term, Block* oldBlock, Block* newBlock);
Block* translate_block_across_blocks(Block* block, Block* oldBlock, Block* newBlock);
void translate_stack_across_blocks(Stack* stack, Block* oldBlock, Block* newBlock);

void update_all_code_references_in_value(caValue* value, Block* oldBlock, Block* newBlock);

void update_world_after_module_reload(World* world, Block* oldBlock, Block* newBlock);


} // namespace circa
