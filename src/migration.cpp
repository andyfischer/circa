// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "code_iterators.h"
#include "interpreter.h"
#include "kernel.h"
#include "migration.h"
#include "term.h"
#include "term_map.h"
#include "world.h"
#include "value_iterator.h"

namespace circa {

void update_all_code_references(Block* target, Block* oldBlock, Block* newBlock)
{
    ca_assert(target != oldBlock);
    ca_assert(target != newBlock);

    // Store a cache of lookups that we've made in this call.
    TermMap cache;

    for (BlockIterator it(target); it.unfinished(); it.advance()) {

        Term* term = *it;

        // Iterate through each "dependency", which includes the function & inputs.
        for (int i=0; i < term->numDependencies(); i++) {
            Term* ref = term->dependency(i);
            Term* newRef = NULL;

            if (cache.contains(ref)) {
                newRef = cache[ref];
            } else {

                // Lookup and save result in cache
                newRef = translate_term_across_blocks(ref, oldBlock, newBlock);
                cache[ref] = newRef;
            }

            // Possibly rebind
            if (newRef != ref)
                term->setDependency(i, newRef);
        }
    }
}

// Returns the corresponding term inside newBlock, if found.
// Returns 'term' if the translation does not apply (term is not found inside
// oldBlock).
// Returns NULL if the translation does apply, but a corresponding term cannot be found.
Term* translate_term_across_blocks(Term* term, Block* oldBlock, Block* newBlock)
{
    if (!term_is_child_of_block(term, oldBlock))
        return term;

    Value relativeName;
    get_relative_name_as_list(term, oldBlock, &relativeName);
    return find_from_relative_name_list(&relativeName, newBlock);
}

// Like translate_term_across_blocks, but for block references.
Block* translate_block_across_blocks(Block* block, Block* oldBlock, Block* newBlock)
{
    // If this is just a reference to 'oldBlock' then simply update it to 'newBlock'.
    if (block == oldBlock)
        return newBlock;

    // Noop on null block.
    if (block == NULL)
        return block;

    // Noop if block has no owner.
    Term* owningTerm = block->owningTerm;
    if (owningTerm == NULL)
        return block;

    // Use owning term to possibly translate a block that is nested inside oldBlock.
    Term* newTerm = translate_term_across_blocks(owningTerm, oldBlock, newBlock);

    if (newTerm == NULL)
        // Deliberate translation to NULL.
        return NULL;
    else
        return newTerm->nestedContents;
}

void translate_stack_across_blocks(Stack* stack, Block* oldBlock, Block* newBlock)
{
    Frame* frame = top_frame(stack);

    while (frame != NULL) {
        frame->block = translate_block_across_blocks(frame->block, oldBlock, newBlock);

        caValue* registers = frame_registers(frame);
        update_all_code_references_in_value(registers, oldBlock, newBlock);

        frame = frame_parent(frame);
    }
}

void update_all_code_references_in_value(caValue* value, Block* oldBlock, Block* newBlock)
{
    for (ValueIterator it(value); it.unfinished(); it.advance()) {
        caValue* val = *it;
        if (is_ref(val)) {
            set_term_ref(val, translate_term_across_blocks(as_term_ref(val),
                oldBlock, newBlock));
            
        } else if (is_block(val)) {
            set_block(val, translate_block_across_blocks(as_block(val),
                        oldBlock, newBlock));
        } else if (is_stack(val)) {
            translate_stack_across_blocks(as_stack(val), oldBlock, newBlock);
        }
    }
}

void update_block_after_module_reload(Block* target, Block* oldBlock, Block* newBlock)
{
    // Noop if the target is our new block
    if (target == newBlock)
        return;

    ca_assert(target != oldBlock);

    update_all_code_references(target, oldBlock, newBlock);
}

void update_world_after_module_reload(World* world, Block* oldBlock, Block* newBlock)
{
    // Update references in every module.
    for (BlockIteratorFlat it(world->root); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.module)
            update_block_after_module_reload(term->nestedContents, oldBlock, newBlock);
    }

    // Update references in every root stack.
    Stack* rootStack = world->firstRootStack;
    while (rootStack != NULL) {
        translate_stack_across_blocks(rootStack, oldBlock, newBlock);
        rootStack = rootStack->nextRootStack;
    }
}


} // namespace circa
