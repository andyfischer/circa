// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "interpreter.h"
#include "hashtable.h"
#include "kernel.h"
#include "list.h"
#include "migration.h"
#include "term.h"
#include "term_map.h"
#include "type.h"
#include "world.h"
#include "value_iterator.h"

namespace circa {

void migrate_block(Block* target, Migration* migration)
{
    ca_assert(target != migration->oldBlock);

    // Migrate term values; needs to happen whether or not the migration targets
    // this block. Needed for ModuleRef values that are written by require().
    // Future: Might be able to change require() to make this unnecessary.
    for (BlockIterator it(target); it.unfinished(); it.advance()) {
        Term* term = *it;
        migrate_value(term_value(term), migration);
    }

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

Type* migrate_type(Type* type, Migration* migration)
{
    if (type->declaringTerm == NULL)
        return NULL;

    Term* newTerm = migrate_term_pointer(type->declaringTerm, migration);
    if (newTerm == NULL)
        return NULL;

    if (newTerm == type->declaringTerm)
        return type;

    if (!is_type(newTerm))
        return NULL;

    return as_type(newTerm);
}

void migrate_state_list(caValue* list, Block* oldBlock, Block* newBlock, Migration* migration)
{
    if (is_null(list))
        return;

    Value oldList;
    move(list, &oldList);
    touch(&oldList);
    set_list(list, newBlock->length());

    // Copy state values with matching names.
    for (int i=0; i < list_length(&oldList); i++) {
        caValue* oldValue = list_get(&oldList, i);
        if (is_null(oldValue))
            continue;

        Term* oldTerm = oldBlock->get(i);
        Term* newTerm = find_from_unique_name(newBlock, unique_name(oldTerm));

#if 0
        printf("looking at unique name = %s for term %d, newTerm = %p\n",
            as_cstring(unique_name(oldTerm)), oldTerm->id, newTerm);
#endif

        if (newTerm != NULL) {
            caValue* newSlot = list_get(list, newTerm->index);
            move(oldValue, newSlot);

#if 0
            printf("migrating state value, term name = %s\n", as_cstring(unique_name(oldTerm)));
#endif
        }
    }

    migrate_value(list, migration);
}

void migrate_stack(Stack* stack, Migration* migration)
{
#if 0
    printf("migrating stack #%d\n", stack->id);
#endif

    stack_on_migration(stack);

    Frame* frame = stack_top(stack);

    while (frame != NULL) {

        Block* oldBlock = frame->block;
        frame->block = migrate_block_pointer(frame->block, migration);

#if 0
        printf("migrate_stack looking at frame with oldBlock #%d, newBlock = #%d\n",
            oldBlock->id, frame->block->id);
#endif

        if (frame->block != NULL && frame->block != oldBlock)
            list_resize(frame_registers(frame), block_locals_count(frame->block));

        migrate_value(frame_registers(frame), migration);
        migrate_value(&frame->bindings, migration);
        migrate_value(&frame->dynamicScope, migration);
        migrate_state_list(&frame->state, oldBlock, frame->block, migration);
        migrate_state_list(&frame->outgoingState, oldBlock, frame->block, migration);

        frame = frame_parent(frame);
    }
}

void migrate_retained_frame(caValue* retainedFrame, Migration* migration)
{
    Block* block = as_block(retained_frame_get_block(retainedFrame));
    Block* newBlock = migrate_block_pointer(block, migration);
    
    if (newBlock == NULL) {
        set_null(retainedFrame);
        return;
    }

    touch(retainedFrame);
    caValue* state = retained_frame_get_state(retainedFrame);
    migrate_state_list(state, block, newBlock, migration);
    set_block(retained_frame_get_block(retainedFrame), newBlock);
}

void migrate_list_value(caValue* value, Migration* migration)
{
    touch(value);
    for (int i=0; i < list_length(value); i++) {
        caValue* element = list_get(value, i);
        migrate_value(element, migration);
    }

    // Migrate type if this is a user-type instance.
    // Future: We could try to reshape the value if the new type has different fields.
    Type* newType = migrate_type(value->value_type, migration);

    if (newType != value->value_type) {
        if (newType == NULL)
            newType = TYPES.list;
        else
            type_incref(newType);

        type_decref(value->value_type);
        value->value_type = newType;
    }
}

void migrate_value(caValue* value, Migration* migration)
{
    if (is_ref(value)) {
        set_term_ref(value, migrate_term_pointer(as_term_ref(value), migration));
        
    } else if (is_block(value)) {
        set_block(value, migrate_block_pointer(as_block(value), migration));
    } else if (is_stack(value)) {
        migrate_stack(as_stack(value), migration);
    } else if (value->value_type == TYPES.mutable_type) {
        caValue* boxedValue = (caValue*) object_get_body(value);
        migrate_value(boxedValue, migration);
    } else if (is_retained_frame(value)) {
        migrate_retained_frame(value, migration);
    } else if (is_list(value)) {
        migrate_list_value(value, migration);

    } else if (is_hashtable(value)) {
        Value oldHashtable;
        move(value, &oldHashtable);

        set_hashtable(value);
        caValue* hashtable = value;

        for (int i=0; i < hashtable_slot_count(&oldHashtable); i++) {
            caValue* oldKey = hashtable_key_by_index(&oldHashtable, i);
            if (oldKey == NULL || is_null(oldKey))
                continue;

            caValue* oldValue = hashtable_value_by_index(&oldHashtable, i);

            Value key;
            copy(oldKey, &key);
            migrate_value(&key, migration);
            caValue* newValue = hashtable_insert(hashtable, &key);
            copy(oldValue, newValue);
            migrate_value(newValue, migration);
        }
    }
}

void migrate_world(World* world, Migration* migration)
{
#if 0
    printf("running migrate_world, from = #%d, to = #%d\n",
        migration->oldBlock->id, migration->newBlock->id);
#endif

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

    world->globalScriptVersion++;
}

Term* migrate_term_pointer(Term* term, Block* oldBlock, Block* newBlock)
{
    Migration migration;
    migration.oldBlock = oldBlock;
    migration.newBlock = newBlock;
    return migrate_term_pointer(term, &migration);
}

Type* migrate_type(Type* type, Block* oldBlock, Block* newBlock)
{
    Migration migration;
    migration.oldBlock = oldBlock;
    migration.newBlock = newBlock;
    return migrate_type(type, &migration);
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
