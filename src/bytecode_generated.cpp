// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "bytecode_generated.h"

const char* bytecode_op_name(char op)
{
    switch (op) {
    case op_NoOp: return "NoOp";
    case op_SetNull: return "SetNull";
    case op_InlineCopy: return "InlineCopy";
    case op_PushBranch: return "PushBranch";
    case op_FireNative: return "FireNative";
    case op_CaseBlock: return "CaseBlock";
    case op_ForLoop: return "ForLoop";
    case op_ErrorNotEnoughInputs: return "ErrorNotEnoughInputs";
    case op_ErrorTooManyInputs: return "ErrorTooManyInputs";
    default: return "(unknown op)";
    }
}
