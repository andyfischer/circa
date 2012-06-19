// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

const char op_Finish = 1;
const char op_Break = 2;

const char op_SetNull = 3;
const char op_Copy = 4;
const char op_CopyOutput = 12;
const char op_CopyInputPlaceholder = 13;
const char op_Cast = 8;

const char op_PushFrame = 10;
const char op_PushNestedFrame = 5;
const char op_PushFunctionFrame = 9;
const char op_FireNative = 11;

const char op_ErrorNotEnoughInputs = 6;
const char op_ErrorTooManyInputs = 7;

struct BytecodeData {
    char* data;
    int writePos;
    int dataCapacity;

    BytecodeData()
      : data(NULL),
        writePos(0),
        dataCapacity(0)
    {}
};

struct BytecodeSlot {
    int frameDelta;
    int registerIndex;
};

bool bytecode_eof(BytecodeData* data, int pos);
int bytecode_advance(BytecodeData* data, int pos);
void bytecode_op_to_string(BytecodeData* data, int pos, caValue* str);

void bytecode_reset(BytecodeData* data);
void bytecode_write_break(BytecodeData* data);

} // namespace circa
