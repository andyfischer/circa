// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

const char bc_End = 0;
const char bc_Pause = 0x1;
const char bc_SetNull = 0x2;
const char bc_InlineCopy = 0x3;
const char bc_LocalCopy = 0x4;
const char bc_NoOp = 0x5;
const char bc_EnterFrame = 0x7;
const char bc_PopFrame = 0x9;
const char bc_DoneTransient = 0xa;
const char bc_FireNative = 0xb;
const char bc_FinishBlock = 0x6;
const char bc_FinishIteration = 0xf;
const char bc_FinishDemandFrame = 0xc;
const char bc_PopRequire = 0xd;
const char bc_SaveInModuleFrames = 0xe;

// Pushing a new frame.
const char bc_PushFunction = 0x10;
const char bc_PushNested = 0x11;
const char bc_PushDynamicMethod = 0x12;
const char bc_PushFuncCall = 0x13;
const char bc_PushFuncApply = 0x15;
const char bc_PushCase = 0x16;
const char bc_PushLoop = 0x17;
const char bc_PushWhile = 0x18;
const char bc_PushRequire = 0x19;

// Pushing inputs to a new frame.
const char bc_InputFromStack = 0x2a;
const char bc_PushValue = 0x2e;
const char bc_InputNull = 0x2b;
const char bc_InputFromValue = 0x2c;
const char bc_PushNonlocalInput = 0x25;
const char bc_InputFromBlockRef = 0x26;
const char bc_PushExplicitState = 0x27;
const char bc_ErrorNotEnoughInputs = 0x28;
const char bc_ErrorTooManyInputs = 0x29;

// Popping outputs from a finished frame.
const char bc_PopOutput = 0x30;
const char bc_PopOutputNull = 0x31;
const char bc_PopOutputsDynamic = 0x32;
const char bc_PopExplicitState = 0x33;

// Control flow.
const char bc_ExitPoint = 0x40;
const char bc_Return = 0x41;
const char bc_Continue = 0x42;
const char bc_Break = 0x43;
const char bc_Discard = 0x44;
const char bc_CaseConditionBool = 0x45;
const char bc_LoopConditionBool = 0x46;
const char bc_Loop = 0x47;

// Memoization
const char bc_MemoizeCheck = 0x50;
const char bc_MemoizeSave = 0x51;

// Inline state.
const char bc_PackState = 0x55;

void bytecode_to_string(caValue* bytecode, caValue* string);
int bytecode_op_to_term_index(const char* bc, int pc);
void bytecode_op_to_string(const char* bc, int* pc, caValue* string);
void bytecode_to_string_lines(caValue* bytecode, caValue* lines);
void bytecode_dump_next_op(const char* bc, int pc);
void bytecode_dump(caValue* bytecode);

void bytecode_write_term_call(caValue* bytecode, Term* term);
void bytecode_write_input_instructions(caValue* bytecode, Term* caller);
void bytecode_write_block(caValue* bytecode, Block* block);

} // namespace circa
