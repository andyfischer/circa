// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

const char bc_Pause = 1;
const char bc_SetNull = 2;
const char bc_InlineCopy = 3;
const char bc_NoOp = 5;
const char bc_Done = 6;
const char bc_EnterFrame = 7;
const char bc_LeaveFrame = 8;
const char bc_PopFrame = 9;
const char bc_DoneTransient = 0xa;
const char bc_FireNative = 0x15;
const char bc_LoopDone = 0x1d;
const char bc_ErrorNotEnoughInputs = 0x1e;
const char bc_ErrorTooManyInputs = 0x1f;

// Pushing a new frame.
const char bc_PushFunction = 0x10;
const char bc_PushNested = 0x11;
const char bc_PushDynamicMethod = 0x12;
const char bc_PushFuncCall = 0x13;
const char bc_PushFuncApply = 0x14;
const char bc_PushCase = 0x16;
const char bc_PushLoop = 0x17;

// Pushing inputs to a new frame.
const char bc_PushInputFromStack = 0x22;
const char bc_PushVarargList = 0x20;
const char bc_PushInputNull = 0x24;
const char bc_PushInputsDynamic = 0x27;
const char bc_PushExplicitState = 0x28;
const char bc_NotEnoughInputs = 0x25;
const char bc_TooManyInputs = 0x26;

// Popping outputs from a finished frame.
const char bc_PopOutput = 0x30;
const char bc_PopOutputNull = 0x31;
const char bc_PopOutputsDynamic = 0x32;
const char bc_PopExplicitState = 0x33;

// Control flow.
const char bc_ExitPoint = 0x18;
const char bc_Return = 0x19;
const char bc_Continue = 0x1a;
const char bc_Break = 0x1b;
const char bc_Discard = 0x1c;

// Memoization
const char bc_UseMemoizedOnEqualInputs = 0x40;
const char bc_MemoizeFrame = 0x41;

// Inline state.
const char bc_UnpackState = 0x50;
const char bc_PackState = 0x51;

void bytecode_to_string(caValue* bytecode, caValue* string);
void bytecode_to_string_lines(caValue* bytecode, caValue* lines);
void bytecode_dump_next_op(caValue* bytecode, Block* block, int pos);
void bytecode_dump(caValue* bytecode);

void bytecode_write_term_call(caValue* bytecode, Term* term);
void bytecode_write_input_instructions(caValue* bytecode, Term* caller, Block* block);
void bytecode_write_block(caValue* bytecode, Block* block);

} // namespace circa
