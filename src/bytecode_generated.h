// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

const char op_NoOp = 0;
const char op_SetNull = 1;
const char op_InlineCopy = 2;
const char op_PushFunctionBranch = 3;
const char op_PushNestedBranch = 4;
const char op_PushBranch = 5;
const char op_FireNative = 6;
const char op_CaseBlock = 7;
const char op_ForLoop = 8;
const char op_ErrorNotEnoughInputs = 9;
const char op_ErrorTooManyInputs = 10;

const char* bytecode_op_name(char op);
