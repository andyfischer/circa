// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct Migration {
    Block* oldBlock;
    Block* newBlock;

    Migration() : oldBlock(NULL), newBlock(NULL) {}
};

// Migrate (possibly modify) all code references inside the block.
void migrate_block(Block* block, Migration* migration);

// Returns a term pointer post-migration. May return the same term (if the migration
// does not apply to this term), or may return NULL if the original term was deleted.
Term* migrate_term_pointer(Term* term, Migration* migration);

// Returns a block pointer post-migration. See migrate_term_pointer comments.
Block* migrate_block_pointer(Block* block, Migration* migration);

// Migrate (possibly modify) all code references inside the stack.
void migrate_stack(Stack* stack, Migration* migration);

void migrate_retained_frame(caValue* retainedFrame, Migration* migration);

// Migrate (possibly modify) all code references inside the value.
void migrate_value(caValue* value, Migration* migration);

// Migrate (possibly modify) all code references inside every module and every root stack
// in the world.
void migrate_world(World* world, Migration* migration);

// Convenience functions.
Term* migrate_term_pointer(Term* term, Block* oldBlock, Block* newBlock);
void migrate_block(Block* block, Block* oldBlock, Block* newBlock);
void migrate_stack(Stack* stack, Block* oldBlock, Block* newBlock);
void migrate_value(caValue* value, Block* oldBlock, Block* newBlock);

} // namespace circa
