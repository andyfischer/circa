// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "hashtable.h"
#include "kernel.h"
#include "list.h"
#include "migration.h"
#include "term.h"
#include "term_map.h"
#include "type.h"
#include "world.h"
#include "value_iterator.h"
#include "vm.h"

namespace circa {

bool value_may_need_migration(Value* value, Migration* migration);
bool list_value_may_need_migration(Value* value, Migration* migration);

bool value_may_need_migration(Value* value, Migration* migration)
{
    if (is_list_based(value))
        return list_value_may_need_migration(value, migration);

    if (is_hashtable(value))
        return true; // todo

    if (is_leaf_value(value))
        return false;

    return true;
}

void migrate_block(Block* target, Migration* migration)
{
    ca_assert(target != migration->oldBlock);

    if (target == migration->newBlock)
        return;

    // Store a cache of lookups that we've made in this call.
    TermMap cache;

    for (BlockIterator it(target); it; ++it) {

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

void migrate_state_list(Value* list, Block* oldBlock, Block* newBlock, Migration* migration)
{
    if (is_null(list))
        return;
    if (newBlock == NULL) {
        set_null(list);
        return;
    }

    Value oldList;
    move(list, &oldList);
    touch(&oldList);
    set_list(list, newBlock->length());

    // Copy state values with matching names.
    for (int i=0; i < list_length(&oldList); i++) {
        Value* oldValue = list_get(&oldList, i);
        if (is_null(oldValue))
            continue;

        Term* oldTerm = oldBlock->get(i);
        Term* newTerm = find_from_unique_name(newBlock, unique_name(oldTerm));

#if 0
        printf("looking at unique name = %s for term %d, newTerm = %p\n",
            as_cstring(unique_name(oldTerm)), oldTerm->id, newTerm);
#endif

        if (newTerm != NULL) {
            Value* newSlot = list_get(list, newTerm->index);
            move(oldValue, newSlot);

#if 0
            printf("migrating state value, term name = %s\n", as_cstring(unique_name(oldTerm)));
#endif
        }
    }

    migrate_value(list, migration);
}

void migrate_vm(VM* vm, Migration* migration)
{
    vm_on_code_change(vm);
    migrate_value(&vm->state, migration);

    Block* newMain = migrate_block_pointer(vm->mainBlock, migration);

    if (newMain != vm->mainBlock)
        vm_change_main(vm, newMain);
}

bool list_value_may_need_migration(Value* value, Migration* migration)
{
    for (int i=0; i < list_length(value); i++)
        if (value_may_need_migration(value->index(i), migration))
            return true;

    Type* newType = migrate_type(value->value_type, migration);
    if (newType != value->value_type)
        return true;

    return false;
}

void migrate_list_value(Value* value, Migration* migration)
{
    if (!list_value_may_need_migration(value, migration))
        return;

    touch(value);

    for (int i=0; i < list_length(value); i++) {
        Value* element = list_get(value, i);
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

void migrate_value(Value* value, Migration* migration)
{
    if (is_term_ref(value)) {
        set_term_ref(value, migrate_term_pointer(as_term_ref(value), migration));
        
    } else if (is_block(value)) {
        set_block(value, migrate_block_pointer(as_block(value), migration));
    } else if (is_list_based(value)) {
        migrate_list_value(value, migration);

    } else if (is_hashtable(value)) {
        Value oldHashtable;
        move(value, &oldHashtable);

        set_hashtable(value);
        Value* hashtable = value;

        for (int i=0; i < hashtable_slot_count(&oldHashtable); i++) {
            Value* oldKey = hashtable_key_by_index(&oldHashtable, i);
            if (oldKey == NULL || is_null(oldKey))
                continue;

            Value* oldValue = hashtable_value_by_index(&oldHashtable, i);

            Value key;
            copy(oldKey, &key);
            migrate_value(&key, migration);
            Value* newValue = hashtable_insert(hashtable, &key);
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

    for (VM* vm = world->firstLiveVM; vm != NULL; vm = vm->nextLiveVM)
        migrate_vm(vm, migration);

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

void migrate_vm(VM* vm, Block* oldBlock, Block* newBlock)
{
    Migration migration;
    migration.oldBlock = oldBlock;
    migration.newBlock = newBlock;
    return migrate_vm(vm, &migration);
}

void migrate_value(Value* value, Block* oldBlock, Block* newBlock)
{
    Migration migration;
    migration.oldBlock = oldBlock;
    migration.newBlock = newBlock;
    return migrate_value(value, &migration);
}

} // namespace circa
