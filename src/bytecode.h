// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Instructions
const char bc_End = 0x0;
const char bc_Noop = 0xf;
const char bc_Pause = 0x1;
const char bc_InlineCopy = 0x3;
const char bc_LocalCopy = 0x4;
const char bc_NoOp = 0x6;
const char bc_PopFrame = 0x7;
const char bc_PopFrameAndPause = 0x8;
const char bc_NativeCall = 0xa;
const char bc_FinishFrame = 0xc;

const char bc_PushFrame = 0x20;
const char bc_PrepareBlock = 0xc2;
const char bc_PrepareBlockUncompiled = 0xc8;
const char bc_EnterFrame = 0xc1;
const char bc_EnterFrameNext = 0xc3;
const char bc_ResolveClosureCall = 0xc4;
const char bc_ResolveClosureApply = 0xc6;
const char bc_ResolveDynamicMethod = 0x12;
const char bc_ResolveDynamicFuncToClosureCall = 0xcf;

// Inputs
const char bc_FoldIncomingVarargs = 0x38;
const char bc_CheckInputs = 0x39;
const char bc_CopyTermValue = 0x21;
const char bc_CopyStackValue = 0xc5;
const char bc_MoveStackValue = 0x3a;
const char bc_CopyCachedValue = 0x2d;
const char bc_SetNull = 0x2b;
const char bc_SetZero = 0x2c;
const char bc_SetEmptyList = 0x3b;
const char bc_Increment = 0x2f;
const char bc_MoveAppend = 0xd0;
const char bc_SetTermRef = 0x2e;
const char bc_ErrorNotEnoughInputs = 0x28;
const char bc_ErrorTooManyInputs = 0x29;

// Popping outputs from a finished frame
const char bc_PopOutput = 0x30;
const char bc_PopOutputNull = 0x31;
const char bc_PopOutputsDynamic = 0x32;
const char bc_SetFrameOutput = 0x34;

// Control flow
const char bc_Jump = 0x4c;
const char bc_JumpIf = 0x45;
const char bc_JumpIfIteratorDone = 0x46;
const char bc_JumpToLoopStart = 0x4d;

// Raw values
const char bc_SetInt = 0x55;
const char bc_SetFloat = 0x56;

// Math
const char bc_Addf = 0x60;
const char bc_Addi = 0x61;
const char bc_Subf = 0x62;
const char bc_Subi = 0x63;
const char bc_Multf = 0x64;
const char bc_Multi = 0x65;
const char bc_Divf = 0x66;
const char bc_Divi = 0x67;
const char bc_Eqf = 0x68;
const char bc_Neqf = 0x69;
const char bc_EqShallow = 0x6a;
const char bc_NeqShallow = 0x6b;

// Other
const char bc_WatchCheck = 0x75;
const char bc_NoteBlockStart = 0x76;
const char bc_Comment = 0x77;

// Method lookup cache
struct MethodCallSiteCacheLine {
    u32 typeId;
    u32 blockIndex;
};

const int c_methodCacheCount = 5;
const size_t c_methodCacheSize = c_methodCacheCount * sizeof(MethodCallSiteCacheLine);

struct CompiledBlock {
    Block* block;
    int bytecodeOffset;
    bool compileInProgress;
    bool hasWatch;
    Symbol hasState; // [:No :Yes :Unknown :Maybe]
};

struct Compiled
{
    World* world;

    Value bytecode; // blob
    
    CompiledBlock* blocks;
    int blockCount;
    Value blockMap; // Map of Block* to CompiledBlock index.

    // Hackset that was used in the generation of this bytecode.
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

void bytecode_to_string(caValue* bytecode, caValue* string);
int bytecode_op_to_term_index(const char* bc, int pc);
void bytecode_op_to_string(const char* bc, int* pc, caValue* string);
void bytecode_to_string_lines(char* bytecode, caValue* lines);
void bytecode_dump_next_op(const char* bc, int* pc);
void bytecode_dump_val(caValue* bytecode);
void bytecode_dump(char* data);

void bytecode_write_term_call(Compiled* compiled, caValue* bytecode, Term* term);

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
void program_erase(Compiled* compiled);

void compiled_to_string(Compiled* compiled, Value* out);
void compiled_dump(Compiled* compiled);

} // namespace circa
