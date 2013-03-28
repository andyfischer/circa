// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void reflection_install_functions(Block* kernel);

Term* translate_term_across_blocks(Term* term, Block* oldBlock, Block* newBlock);
Block* translate_block_across_blocks(Block* block, Block* oldBlock, Block* newBlock);
void translate_stack_across_blocks(Stack* stack, Block* oldBlock, Block* newBlock);

void update_all_code_references_in_value(caValue* value, Block* oldBlock, Block* newBlock);

} // namespace circa
