// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct ProgramBlock {
    // Per-block data stored in a Program.
    
    Block* block;
    Value bytecode;
    bool hasWatch;
};

struct Program
{
    // Cached information related to running a stack. All of this data is per-stack, and
    // is erased when the hackset changes.
    
    ProgramBlock* blocks;
    int blockCount;

    Value indexMap; // Map of Block* to block index.

    // Hackset that was used in the generation of this program.
    Value hackset;

    // Additional information.
    bool skipEffects;
    bool noSaveState;

    Value hacksByTerm;

    // Map of watch key to cachedValue index (containing the watch).
    Value watchByKey;

    // List of values that can be referenced by InputFromCachedValue.
    Value cachedValues;
};

Program* alloc_program();
void free_program(Program* program);

char* program_block_bytecode(Program* program, int index);
Block* program_block(Program* program, int index);
ProgramBlock* program_block_info(Program* program, int index);
int program_find_block_index(Program* program, Block* block);
void program_generate_bytecode(Program* program, int blockIndex);
int program_create_empty_entry(Program* program, Block* block);
int program_create_entry(Program* program, Block* block);
caValue* program_add_cached_value(Program* program, int* index);
void program_add_watch(Program* program, caValue* key, caValue* path);
caValue* program_get_watch_observation(Program* program, caValue* key);
void program_set_hackset(Program* program, caValue* hackset);
void program_erase(Program* program);

} // namespace circa
