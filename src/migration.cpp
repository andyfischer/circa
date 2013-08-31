// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "interpreter.h"
#include "kernel.h"
#include "migration.h"
#include "term.h"
#include "term_map.h"
#include "world.h"
#include "value_iterator.h"

namespace circa {

void migrate_block(Block* target, Migration* migration)
{
    ca_assert(target != migration->oldBlock);

    if (target == migration->newBlock)
        return;

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
                newRef = migrate_term_pointer(ref, migration);
                cache[ref] = newRef;
            }

            // Possibly rebind
            if (newRef != ref)
                term->setDependency(i, newRef);
        }
    }
}

Term* migrate_term_pointer(Term* term, Migration* migration)
{
    if (!term_is_child_of_block(term, migration->oldBlock))
        return term;

    Value relativeName;
    get_relative_name_as_list(term, migration->oldBlock, &relativeName);
    return find_from_relative_name_list(&relativeName, migration->newBlock);
}

Block* migrate_block_pointer(Block* block, Migration* migration)
{
    // If this is just a reference to 'oldBlock' then simply update it to 'newBlock'.
    if (block == migration->oldBlock)
        return migration->newBlock;

    // No-op on null block.
    if (block == NULL)
        return block;

    // No-op if block has no owner.
    Term* owningTerm = block->owningTerm;
    if (owningTerm == NULL)
        return block;

    // Use owning term to possibly translate a block that is nested inside oldBlock.
    Term* newTerm = migrate_term_pointer(owningTerm, migration);

    if (newTerm == NULL)
        // Deliberate translation to NULL.
        return NULL;
    else
        return newTerm->nestedContents;
}

void migrate_stack(Stack* stack, Migration* migration)
{
    Frame* frame = stack_top(stack);

    while (frame != NULL) {
        // Save state output
        Value stateOutput;
        Term* stateOutputTerm = find_state_output(frame->block);
        if (stateOutputTerm != NULL)
            move(frame_register(frame, stateOutputTerm), &stateOutput);

        frame->block = migrate_block_pointer(frame->block, migration);

        if (frame->block != NULL) {

            list_resize(frame_registers(frame), block_locals_count(frame->block));

            if (stateOutputTerm != NULL) {
                Term* newStateOutputTerm = find_state_output(frame->block);
                if (newStateOutputTerm != NULL)
                    move(&stateOutput, frame_register(frame, newStateOutputTerm));
            }
        }

        migrate_value(frame_registers(frame), migration);

        frame = frame_parent(frame);
    }
}

void migrate_value(caValue* value, Migration* migration)
{
    if (is_list(value)) {
        for (ValueIterator it(value); it.unfinished(); it.advance()) {
            caValue* element = *it;
            migrate_value(element, migration);
        }
    } else if (is_ref(value)) {
        set_term_ref(value, migrate_term_pointer(as_term_ref(value), migration));
        
    } else if (is_block(value)) {
        set_block(value, migrate_block_pointer(as_block(value), migration));
    } else if (is_stack(value)) {
        migrate_stack(as_stack(value), migration);
    } else if (value->value_type == TYPES.mutable_type) {
        caValue* boxedValue = (caValue*) object_get_body(value);
        migrate_value(boxedValue, migration);
    }
}

void migrate_world(World* world, Migration* migration)
{
    // Update references in every module.
    for (BlockIteratorFlat it(world->root); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.module)
            migrate_block(term->nestedContents, migration);
    }

    // Update references in every stack.
    Stack* rootStack = world->firstStack;
    while (rootStack != NULL) {
        migrate_stack(rootStack, migration);
        rootStack = rootStack->nextStack;
    }
}

Term* migrate_term_pointer(Term* term, Block* oldBlock, Block* newBlock)
{
    Migration migration;
    migration.oldBlock = oldBlock;
    migration.newBlock = newBlock;
    return migrate_term_pointer(term, &migration);
}
void migrate_block(Block* block, Block* oldBlock, Block* newBlock)
{
    Migration migration;
    migration.oldBlock = oldBlock;
    migration.newBlock = newBlock;
    return migrate_block(block, &migration);
}

void migrate_stack(Stack* stack, Block* oldBlock, Block* newBlock)
{
    Migration migration;
    migration.oldBlock = oldBlock;
    migration.newBlock = newBlock;
    return migrate_stack(stack, &migration);
}

void migrate_value(caValue* value, Block* oldBlock, Block* newBlock)
{
    Migration migration;
    migration.oldBlock = oldBlock;
    migration.newBlock = newBlock;
    return migrate_value(value, &migration);
}

} // namespace circa
