// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#pragma once

namespace circa {

const int name_None = 0;
const int name_File = 1;
const int name_Newline = 2;
const int name_Out = 3;
const int name_Unknown = 4;
const int name_Repeat = 5;
const int name_Success = 6;
const int name_Failure = 7;
const int name_FileNotFound = 8;
const int name_NotEnoughInputs = 9;
const int name_TooManyInputs = 10;
const int name_ExtraOutputNotFound = 11;
const int name_Default = 12;
const int name_ByDemand = 13;
const int name_Unevaluated = 14;
const int name_InProgress = 15;
const int name_Lazy = 16;
const int name_Consumed = 17;
const int name_Return = 18;
const int name_Continue = 19;
const int name_Break = 20;
const int name_Discard = 21;
const int name_InfixOperator = 22;
const int name_FunctionName = 23;
const int name_TypeName = 24;
const int name_TermName = 25;
const int name_Keyword = 26;
const int name_Whitespace = 27;
const int name_UnknownIdentifier = 28;
const int name_LastBuiltinName = 29;

const char* builtin_name_to_string(int name);

} // namespace circa
