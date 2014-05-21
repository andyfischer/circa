// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "bytecode.h"
#include "hashtable.h"
#include "list.h"
#include "program.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"

namespace circa {

Program* alloc_program()
{
    Program* program = new Program();
    program->blocks = NULL;
    program->blockCount = 0;
    set_hashtable(&program->indexMap);
    set_hashtable(&program->hacksByTerm);
    set_hashtable(&program->watchByKey);
    set_list(&program->cachedValues);
    return program;
}

void free_program(Program* program)
{
    delete program;
}

char* program_block_bytecode(Program* program, int index)
{
    ca_assert(index < program->blockCount);
    return as_blob(&program->blocks[index].bytecode);
}

Block* program_block(Program* program, int index)
{
    ca_assert(index < program->blockCount);
    return program->blocks[index].block;
}

ProgramBlock* program_block_info(Program* program, int index)
{
    ca_assert(index < program->blockCount);
    return &program->blocks[index];
}

int program_find_block_index(Program* program, Block* block)
{
    Value key;
    set_block(&key, block);
    caValue* indexVal = hashtable_get(&program->indexMap, &key);
    if (indexVal == NULL)
        return -1;
    return as_int(indexVal);
}

void program_generate_bytecode(Program* program, int blockIndex)
{
    ProgramBlock* entry = &program->blocks[blockIndex];

    if (!is_null(&entry->bytecode))
        return;

    Value bytecode;
    bytecode_write_block(program, &bytecode, entry->block);

    // Save bytecode. Re-lookup the entry because the above bytecode_write step
    // may have reallocated program->blocks.
    entry = &program->blocks[blockIndex];
    move(&bytecode, &entry->bytecode);
}

int program_create_empty_entry(Program* program, Block* block)
{
    int existing = program_find_block_index(program, block);
    if (existing != -1)
        return existing;

    // Doesn't exist, new create entry.
    int newIndex = program->blockCount++;
    program->blocks = (ProgramBlock*) realloc(program->blocks,
        sizeof(*program->blocks) * program->blockCount);

    ProgramBlock* entry = &program->blocks[newIndex];
    initialize_null(&entry->bytecode);
    entry->block = block;
    entry->hasWatch = false;

    Value key;
    set_block(&key, block);
    set_int(hashtable_insert(&program->indexMap, &key), newIndex);

    return newIndex;
}

int program_create_entry(Program* program, Block* block)
{
    int index = program_create_empty_entry(program, block);
    program_generate_bytecode(program, index);
    return index;
}

caValue* program_add_cached_value(Program* program, int* index)
{
    *index = list_length(&program->cachedValues);
    return list_append(&program->cachedValues);
}

void program_add_watch(Program* program, caValue* key, caValue* path)
{
    int cachedIndex = 0;
    caValue* watch = program_add_cached_value(program, &cachedIndex);

    // Watch value is: [key, path, observation]
    set_list(watch, 3);
    watch->set_element(0, key);
    watch->set_element(1, path);
    watch->set_element_null(2);

    // Update watchByKey
    set_int(hashtable_insert(&program->watchByKey, key), cachedIndex);

    Term* targetTerm = as_term_ref(list_last(path));

    // Update ProgramBlock
    ProgramBlock* blockInfo = program_block_info(program,
        program_create_empty_entry(program, targetTerm->owningBlock));
    blockInfo->hasWatch = true;
}

caValue* program_get_watch_observation(Program* program, caValue* key)
{
    caValue* index = hashtable_get(&program->watchByKey, key);
    if (index == NULL)
        return NULL;

    caValue* watch = list_get(&program->cachedValues, as_int(index));

    return watch->element(2);
}

void program_set_hackset(Program* program, caValue* hackset)
{
    for (int i=0; i < list_length(hackset); i++) {
        caValue* hacksetElement = hackset->element(i);
        if (symbol_eq(hacksetElement, sym_no_effect))
            program->skipEffects = true;
        else if (symbol_eq(hacksetElement, sym_no_save_state))
            program->noSaveState = true;
        else if (first_symbol(hacksetElement) == sym_set_value) {
            caValue* setValueTarget = hacksetElement->element(1);
            caValue* setValueNewValue = hacksetElement->element(2);

            caValue* termHacks = hashtable_insert(&program->hacksByTerm, setValueTarget);
            if (!is_hashtable(termHacks))
                set_hashtable(termHacks);

            int cachedValueIndex = 0;
            set_value(program_add_cached_value(program, &cachedValueIndex), setValueNewValue);
            set_int(hashtable_insert_symbol_key(termHacks, sym_set_value), cachedValueIndex);
        } else if (first_symbol(hacksetElement) == sym_watch) {
            caValue* watchKey = hacksetElement->element(1);
            caValue* watchPath = hacksetElement->element(2);
            program_add_watch(program, watchKey, watchPath);
        }
    }
}

void program_erase(Program* program)
{
    for (int b=0; b < program->blockCount; b++) {
        ProgramBlock* blockEntry = &program->blocks[b];
        set_null(&blockEntry->bytecode);
    }
    free(program->blocks);
    program->blocks = NULL;
    program->blockCount = 0;
    set_hashtable(&program->indexMap);
    program->noSaveState = false;
    program->skipEffects = false;
    set_hashtable(&program->hacksByTerm);
    set_hashtable(&program->watchByKey);
    set_list(&program->cachedValues);
}

} // namespace circa
