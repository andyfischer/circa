// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Instructions
const char bc_End = 0x0;
const char bc_Noop = 0x1;
const char bc_Pause = 0x2;
const char bc_PopFrame = 0x3;
const char bc_PopFrameAndPause = 0x4;
const char bc_NativeCall = 0x5;

const char bc_PushFrame = 0x8;
const char bc_FinishFrame = 0x9;
const char bc_PrepareBlock = 0xa;
const char bc_PrepareBlockUncompiled = 0xb;
const char bc_EnterFrame = 0xc;
const char bc_EnterFrameNext = 0xd;
const char bc_ResolveClosureCall = 0xe;
const char bc_ResolveClosureApply = 0xf;
const char bc_ResolveDynamicMethod = 0x10;
const char bc_ResolveDynamicFuncToClosureCall = 0x11;

// Inputs
const char bc_FoldIncomingVarargs = 0x16;
const char bc_CheckInputs = 0x17;
const char bc_CopyTermValue = 0x18;
const char bc_CopyStackValue = 0x19;
const char bc_MoveStackValue = 0x1a;
const char bc_CopyCachedValue = 0x1b;
const char bc_SetNull = 0x1c;
const char bc_SetZero = 0x1d;
const char bc_SetEmptyList = 0x1e;
const char bc_Increment = 0x1f;
const char bc_AppendMove = 0x20;
const char bc_GetIndexCopy = 0x21;
const char bc_GetIndexMove = 0x22;
const char bc_Touch = 0x26;
const char bc_SetTermRef = 0x23;

// Popping outputs from a finished frame
const char bc_PopOutput = 0x30;
const char bc_PopOutputNull = 0x31;
const char bc_PopOutputsDynamic = 0x32;
const char bc_SetFrameOutput = 0x33;

// Control flow
const char bc_Jump = 0x34;
const char bc_JumpIf = 0x35;
const char bc_JumpIfIteratorDone = 0x36;
const char bc_JumpToLoopStart = 0x37;

// Raw values
const char bc_SetInt = 0x3a;
const char bc_SetFloat = 0x3b;

// Math
const char bc_Addf = 0x40;
const char bc_Addi = 0x41;
const char bc_Subf = 0x42;
const char bc_Subi = 0x43;
const char bc_Multf = 0x44;
const char bc_Multi = 0x45;
const char bc_Divf = 0x46;
const char bc_Divi = 0x47;
const char bc_Eqf = 0x48;
const char bc_Neqf = 0x49;
const char bc_EqShallow = 0x4a;
const char bc_NeqShallow = 0x4b;

// Other
const char bc_WatchCheck = 0x50;
const char bc_NoteBlockStart = 0x51;
const char bc_Comment = 0x52;
const char bc_IncrementTermCounter = 0x53;

// Method lookup cache
const u8 MethodCache_NormalMethod = 1;
const u8 MethodCache_ModuleAccess = 2;
const u8 MethodCache_HashtableGet = 3;
const u8 MethodCache_NotFound = 4;

struct MethodCacheLine {
    u32 typeId;
    u32 blockIndex;
    u8 methodCacheType;
};

const int c_methodCacheCount = 4;
const size_t c_methodCacheSize = c_methodCacheCount * sizeof(MethodCacheLine);

struct CompiledBlock {
    Block* block;
    int bytecodeOffset;
    bool compileInProgress;
    bool hasWatch;

    // Accumulated knowledge
    int* termCounter;
};

struct Compiled
{
    World* world;

    Value bytecode;
    
    CompiledBlock* blocks;
    int blockCount;
    Value blockMap; // Map of Block* to CompiledBlock index.

    // Hackset that was used in the generation of this bytecode.
    Value hackset;

    // Additional information.
    bool skipEffects;
    bool noSaveState;
    bool enableTermCounter;

    Value hacksByTerm;

    // Map of watch key to cachedValue index (containing the watch).
    Value watchByKey;

    // List of values that can be referenced by InputFromCachedValue.
    Value cachedValues;
};

void bytecode_to_string(caValue* bytecode, caValue* string);
int bytecode_op_to_term_index(const char* bc, int pc);
void bytecode_op_to_string(const char* bc, int* pc, caValue* string);
void bytecode_to_string_lines(char* bytecode, caValue* lines);
void bytecode_dump_next_op(const char* bc, int* pc);
void bytecode_dump_val(caValue* bytecode);
void bytecode_dump(char* data);

Compiled* alloc_program(World* world);
void free_program(Compiled* compiled);

Block* program_block(Compiled* compiled, int index);
CompiledBlock* compiled_block(Compiled* compiled, int index);
int program_find_block_index(Compiled* compiled, Block* block);
void program_generate_bytecode(Compiled* compiled, int blockIndex);
int program_create_empty_entry(Compiled* compiled, Block* block);
int program_create_entry(Compiled* compiled, Block* block);
caValue* program_add_cached_value(Compiled* compiled, int* index);
caValue* program_get_cached_value(Compiled* compiled, int index);
void program_add_watch(Compiled* compiled, caValue* key, caValue* path);
caValue* program_get_watch_observation(Compiled* compiled, caValue* key);
void program_set_hackset(Compiled* compiled, caValue* hackset);
void compiled_reset_trace_data(Compiled* compiled);
void program_erase(Compiled* compiled);

void compiled_to_string(Compiled* compiled, Value* out);
void compiled_dump(Compiled* compiled);

} // namespace circa
