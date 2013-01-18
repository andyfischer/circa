// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#include "common_headers.h"
#include "names_builtin.h"

namespace circa {

const char* builtin_name_to_string(int name)
{
    if (name >= name_LastBuiltinName)
        return NULL;

    switch (name) {
    case name_None: return "None";
    case name_Invalid: return "Invalid";
    case name_File: return "File";
    case name_Newline: return "Newline";
    case name_Out: return "Out";
    case name_Unknown: return "Unknown";
    case name_Repeat: return "Repeat";
    case name_Success: return "Success";
    case name_Failure: return "Failure";
    case name_FileNotFound: return "FileNotFound";
    case name_NotEnoughInputs: return "NotEnoughInputs";
    case name_TooManyInputs: return "TooManyInputs";
    case name_ExtraOutputNotFound: return "ExtraOutputNotFound";
    case name_Default: return "Default";
    case name_ByDemand: return "ByDemand";
    case name_Unevaluated: return "Unevaluated";
    case name_InProgress: return "InProgress";
    case name_Lazy: return "Lazy";
    case name_Consumed: return "Consumed";
    case name_Uncaptured: return "Uncaptured";
    case name_Return: return "Return";
    case name_Continue: return "Continue";
    case name_Break: return "Break";
    case name_Discard: return "Discard";
    case name_Control: return "Control";
    case name_ExitLevelFunction: return "ExitLevelFunction";
    case name_ExitLevelLoop: return "ExitLevelLoop";
    case name_HighestExitLevel: return "HighestExitLevel";
    case name_ExtraReturn: return "ExtraReturn";
    case name_Name: return "Name";
    case name_Primary: return "Primary";
    case name_Anonymous: return "Anonymous";
    case name_InfixOperator: return "InfixOperator";
    case name_FunctionName: return "FunctionName";
    case name_TypeName: return "TypeName";
    case name_TermName: return "TermName";
    case name_Keyword: return "Keyword";
    case name_Whitespace: return "Whitespace";
    case name_UnknownIdentifier: return "UnknownIdentifier";
    case name_LookupAny: return "LookupAny";
    case name_LookupType: return "LookupType";
    case name_LookupFunction: return "LookupFunction";
    case name_LookupModule: return "LookupModule";
    case name_Untyped: return "Untyped";
    case name_UniformListType: return "UniformListType";
    case name_AnonStructType: return "AnonStructType";
    case name_StructType: return "StructType";
    case name_NativePatch: return "NativePatch";
    case name_PatchBlock: return "PatchBlock";
    case name_Bootstrapping: return "Bootstrapping";
    case name_Done: return "Done";
    case name_StorageTypeNull: return "StorageTypeNull";
    case name_StorageTypeInt: return "StorageTypeInt";
    case name_StorageTypeFloat: return "StorageTypeFloat";
    case name_StorageTypeBool: return "StorageTypeBool";
    case name_StorageTypeString: return "StorageTypeString";
    case name_StorageTypeList: return "StorageTypeList";
    case name_StorageTypeOpaquePointer: return "StorageTypeOpaquePointer";
    case name_StorageTypeTerm: return "StorageTypeTerm";
    case name_StorageTypeType: return "StorageTypeType";
    case name_StorageTypeHandle: return "StorageTypeHandle";
    case name_StorageTypeHashtable: return "StorageTypeHashtable";
    case name_StorageTypeObject: return "StorageTypeObject";
    case tok_Identifier: return "tok_Identifier";
    case tok_ColonString: return "tok_ColonString";
    case tok_Integer: return "tok_Integer";
    case tok_HexInteger: return "tok_HexInteger";
    case tok_Float: return "tok_Float";
    case tok_String: return "tok_String";
    case tok_Color: return "tok_Color";
    case tok_Bool: return "tok_Bool";
    case tok_LParen: return "tok_LParen";
    case tok_RParen: return "tok_RParen";
    case tok_LBrace: return "tok_LBrace";
    case tok_RBrace: return "tok_RBrace";
    case tok_LBracket: return "tok_LBracket";
    case tok_RBracket: return "tok_RBracket";
    case tok_Comma: return "tok_Comma";
    case tok_At: return "tok_At";
    case tok_Dot: return "tok_Dot";
    case tok_DotAt: return "tok_DotAt";
    case tok_Star: return "tok_Star";
    case tok_Question: return "tok_Question";
    case tok_Slash: return "tok_Slash";
    case tok_DoubleSlash: return "tok_DoubleSlash";
    case tok_Plus: return "tok_Plus";
    case tok_Minus: return "tok_Minus";
    case tok_LThan: return "tok_LThan";
    case tok_LThanEq: return "tok_LThanEq";
    case tok_GThan: return "tok_GThan";
    case tok_GThanEq: return "tok_GThanEq";
    case tok_Percent: return "tok_Percent";
    case tok_Colon: return "tok_Colon";
    case tok_DoubleColon: return "tok_DoubleColon";
    case tok_DoubleEquals: return "tok_DoubleEquals";
    case tok_NotEquals: return "tok_NotEquals";
    case tok_Equals: return "tok_Equals";
    case tok_PlusEquals: return "tok_PlusEquals";
    case tok_MinusEquals: return "tok_MinusEquals";
    case tok_StarEquals: return "tok_StarEquals";
    case tok_SlashEquals: return "tok_SlashEquals";
    case tok_ColonEquals: return "tok_ColonEquals";
    case tok_RightArrow: return "tok_RightArrow";
    case tok_LeftArrow: return "tok_LeftArrow";
    case tok_Ampersand: return "tok_Ampersand";
    case tok_DoubleAmpersand: return "tok_DoubleAmpersand";
    case tok_DoubleVerticalBar: return "tok_DoubleVerticalBar";
    case tok_Semicolon: return "tok_Semicolon";
    case tok_TwoDots: return "tok_TwoDots";
    case tok_Ellipsis: return "tok_Ellipsis";
    case tok_TripleLThan: return "tok_TripleLThan";
    case tok_TripleGThan: return "tok_TripleGThan";
    case tok_Pound: return "tok_Pound";
    case tok_Def: return "tok_Def";
    case tok_Type: return "tok_Type";
    case tok_UnusedName1: return "tok_UnusedName1";
    case tok_UnusedName2: return "tok_UnusedName2";
    case tok_UnusedName3: return "tok_UnusedName3";
    case tok_If: return "tok_If";
    case tok_Else: return "tok_Else";
    case tok_Elif: return "tok_Elif";
    case tok_For: return "tok_For";
    case tok_State: return "tok_State";
    case tok_Return: return "tok_Return";
    case tok_In: return "tok_In";
    case tok_True: return "tok_True";
    case tok_False: return "tok_False";
    case tok_Namespace: return "tok_Namespace";
    case tok_Include: return "tok_Include";
    case tok_And: return "tok_And";
    case tok_Or: return "tok_Or";
    case tok_Not: return "tok_Not";
    case tok_Discard: return "tok_Discard";
    case tok_Null: return "tok_Null";
    case tok_Break: return "tok_Break";
    case tok_Continue: return "tok_Continue";
    case tok_Switch: return "tok_Switch";
    case tok_Case: return "tok_Case";
    case tok_While: return "tok_While";
    case tok_Require: return "tok_Require";
    case tok_Package: return "tok_Package";
    case tok_Section: return "tok_Section";
    case tok_Whitespace: return "tok_Whitespace";
    case tok_Newline: return "tok_Newline";
    case tok_Comment: return "tok_Comment";
    case tok_Eof: return "tok_Eof";
    case tok_Unrecognized: return "tok_Unrecognized";
    case op_NoOp: return "op_NoOp";
    case op_Pause: return "op_Pause";
    case op_SetNull: return "op_SetNull";
    case op_InlineCopy: return "op_InlineCopy";
    case op_CallBlock: return "op_CallBlock";
    case op_DynamicCall: return "op_DynamicCall";
    case op_ClosureCall: return "op_ClosureCall";
    case op_FireNative: return "op_FireNative";
    case op_CaseBlock: return "op_CaseBlock";
    case op_ForLoop: return "op_ForLoop";
    case op_ExitPoint: return "op_ExitPoint";
    case op_Return: return "op_Return";
    case op_Continue: return "op_Continue";
    case op_Break: return "op_Break";
    case op_Discard: return "op_Discard";
    case op_FinishFrame: return "op_FinishFrame";
    case op_FinishLoop: return "op_FinishLoop";
    case op_ErrorNotEnoughInputs: return "op_ErrorNotEnoughInputs";
    case op_ErrorTooManyInputs: return "op_ErrorTooManyInputs";
    case name_LoopProduceOutput: return "LoopProduceOutput";
    case name_FlatOutputs: return "FlatOutputs";
    case name_OutputsToList: return "OutputsToList";
    case name_Multiple: return "Multiple";
    case name_Cast: return "Cast";
    case name_DynamicMethodOutput: return "DynamicMethodOutput";
    case name_FirstStatIndex: return "FirstStatIndex";
    case stat_TermsCreated: return "stat_TermsCreated";
    case stat_TermPropAdded: return "stat_TermPropAdded";
    case stat_TermPropAccess: return "stat_TermPropAccess";
    case stat_InternedNameLookup: return "stat_InternedNameLookup";
    case stat_InternedNameCreate: return "stat_InternedNameCreate";
    case stat_Copy_PushedInputNewFrame: return "stat_Copy_PushedInputNewFrame";
    case stat_Copy_PushedInputMultiNewFrame: return "stat_Copy_PushedInputMultiNewFrame";
    case stat_Copy_PushFrameWithInputs: return "stat_Copy_PushFrameWithInputs";
    case stat_Copy_ListDuplicate: return "stat_Copy_ListDuplicate";
    case stat_Copy_LoopCopyRebound: return "stat_Copy_LoopCopyRebound";
    case stat_Cast_ListCastElement: return "stat_Cast_ListCastElement";
    case stat_Cast_PushFrameWithInputs: return "stat_Cast_PushFrameWithInputs";
    case stat_Cast_FinishFrame: return "stat_Cast_FinishFrame";
    case stat_Touch_ListCast: return "stat_Touch_ListCast";
    case stat_ValueCreates: return "stat_ValueCreates";
    case stat_ValueCopies: return "stat_ValueCopies";
    case stat_ValueCast: return "stat_ValueCast";
    case stat_ValueCastDispatched: return "stat_ValueCastDispatched";
    case stat_ValueTouch: return "stat_ValueTouch";
    case stat_ListsCreated: return "stat_ListsCreated";
    case stat_ListsGrown: return "stat_ListsGrown";
    case stat_ListSoftCopy: return "stat_ListSoftCopy";
    case stat_ListHardCopy: return "stat_ListHardCopy";
    case stat_DictHardCopy: return "stat_DictHardCopy";
    case stat_StringCreate: return "stat_StringCreate";
    case stat_StringDuplicate: return "stat_StringDuplicate";
    case stat_StringResizeInPlace: return "stat_StringResizeInPlace";
    case stat_StringResizeCreate: return "stat_StringResizeCreate";
    case stat_StringSoftCopy: return "stat_StringSoftCopy";
    case stat_StringToStd: return "stat_StringToStd";
    case stat_StepInterpreter: return "stat_StepInterpreter";
    case stat_InterpreterCastOutputFromFinishedFrame: return "stat_InterpreterCastOutputFromFinishedFrame";
    case stat_BlockNameLookups: return "stat_BlockNameLookups";
    case stat_PushFrame: return "stat_PushFrame";
    case stat_LoopFinishIteration: return "stat_LoopFinishIteration";
    case stat_LoopWriteOutput: return "stat_LoopWriteOutput";
    case stat_WriteTermBytecode: return "stat_WriteTermBytecode";
    case stat_DynamicCall: return "stat_DynamicCall";
    case stat_FinishDynamicCall: return "stat_FinishDynamicCall";
    case stat_DynamicMethodCall: return "stat_DynamicMethodCall";
    case stat_SetIndex: return "stat_SetIndex";
    case stat_SetField: return "stat_SetField";
    case name_LastStatIndex: return "LastStatIndex";
    case name_LastBuiltinName: return "LastBuiltinName";
    default: return NULL;
    }
}

int builtin_name_from_string(const char* str)
{
    switch (str[0]) {
    default: return -1;
    case 'A':
    switch (str[1]) {
    default: return -1;
    case 'n':
    switch (str[2]) {
    default: return -1;
    case 'o':
    switch (str[3]) {
    default: return -1;
    case 'n':
    switch (str[4]) {
    default: return -1;
    case 'y':
        if (strcmp(str + 5, "mous") == 0)
            return name_Anonymous;
        break;
    case 'S':
        if (strcmp(str + 5, "tructType") == 0)
            return name_AnonStructType;
        break;
    }
    }
    }
    }
    case 'C':
    switch (str[1]) {
    default: return -1;
    case 'a':
        if (strcmp(str + 2, "st") == 0)
            return name_Cast;
        break;
    case 'o':
    switch (str[2]) {
    default: return -1;
    case 'n':
    switch (str[3]) {
    default: return -1;
    case 's':
        if (strcmp(str + 4, "umed") == 0)
            return name_Consumed;
        break;
    case 't':
    switch (str[4]) {
    default: return -1;
    case 'i':
        if (strcmp(str + 5, "nue") == 0)
            return name_Continue;
        break;
    case 'r':
        if (strcmp(str + 5, "ol") == 0)
            return name_Control;
        break;
    }
    }
    }
    }
    case 'B':
    switch (str[1]) {
    default: return -1;
    case 'y':
        if (strcmp(str + 2, "Demand") == 0)
            return name_ByDemand;
        break;
    case 'r':
        if (strcmp(str + 2, "eak") == 0)
            return name_Break;
        break;
    case 'o':
        if (strcmp(str + 2, "otstrapping") == 0)
            return name_Bootstrapping;
        break;
    }
    case 'E':
    switch (str[1]) {
    default: return -1;
    case 'x':
    switch (str[2]) {
    default: return -1;
    case 'i':
    switch (str[3]) {
    default: return -1;
    case 't':
    switch (str[4]) {
    default: return -1;
    case 'L':
    switch (str[5]) {
    default: return -1;
    case 'e':
    switch (str[6]) {
    default: return -1;
    case 'v':
    switch (str[7]) {
    default: return -1;
    case 'e':
    switch (str[8]) {
    default: return -1;
    case 'l':
    switch (str[9]) {
    default: return -1;
    case 'L':
        if (strcmp(str + 10, "oop") == 0)
            return name_ExitLevelLoop;
        break;
    case 'F':
        if (strcmp(str + 10, "unction") == 0)
            return name_ExitLevelFunction;
        break;
    }
    }
    }
    }
    }
    }
    }
    case 't':
    switch (str[3]) {
    default: return -1;
    case 'r':
    switch (str[4]) {
    default: return -1;
    case 'a':
    switch (str[5]) {
    default: return -1;
    case 'R':
        if (strcmp(str + 6, "eturn") == 0)
            return name_ExtraReturn;
        break;
    case 'O':
        if (strcmp(str + 6, "utputNotFound") == 0)
            return name_ExtraOutputNotFound;
        break;
    }
    }
    }
    }
    }
    case 'D':
    switch (str[1]) {
    default: return -1;
    case 'i':
        if (strcmp(str + 2, "scard") == 0)
            return name_Discard;
        break;
    case 'y':
        if (strcmp(str + 2, "namicMethodOutput") == 0)
            return name_DynamicMethodOutput;
        break;
    case 'e':
        if (strcmp(str + 2, "fault") == 0)
            return name_Default;
        break;
    case 'o':
        if (strcmp(str + 2, "ne") == 0)
            return name_Done;
        break;
    }
    case 'F':
    switch (str[1]) {
    default: return -1;
    case 'i':
    switch (str[2]) {
    default: return -1;
    case 'r':
        if (strcmp(str + 3, "stStatIndex") == 0)
            return name_FirstStatIndex;
        break;
    case 'l':
    switch (str[3]) {
    default: return -1;
    case 'e':
    switch (str[4]) {
    default: return -1;
    case 0:
        if (strcmp(str + 5, "") == 0)
            return name_File;
        break;
    case 'N':
        if (strcmp(str + 5, "otFound") == 0)
            return name_FileNotFound;
        break;
    }
    }
    }
    case 'a':
        if (strcmp(str + 2, "ilure") == 0)
            return name_Failure;
        break;
    case 'u':
        if (strcmp(str + 2, "nctionName") == 0)
            return name_FunctionName;
        break;
    case 'l':
        if (strcmp(str + 2, "atOutputs") == 0)
            return name_FlatOutputs;
        break;
    }
    case 'I':
    switch (str[1]) {
    default: return -1;
    case 'n':
    switch (str[2]) {
    default: return -1;
    case 'P':
        if (strcmp(str + 3, "rogress") == 0)
            return name_InProgress;
        break;
    case 'v':
        if (strcmp(str + 3, "alid") == 0)
            return name_Invalid;
        break;
    case 'f':
        if (strcmp(str + 3, "ixOperator") == 0)
            return name_InfixOperator;
        break;
    }
    }
    case 'H':
        if (strcmp(str + 1, "ighestExitLevel") == 0)
            return name_HighestExitLevel;
        break;
    case 'K':
        if (strcmp(str + 1, "eyword") == 0)
            return name_Keyword;
        break;
    case 'M':
        if (strcmp(str + 1, "ultiple") == 0)
            return name_Multiple;
        break;
    case 'L':
    switch (str[1]) {
    default: return -1;
    case 'a':
    switch (str[2]) {
    default: return -1;
    case 's':
    switch (str[3]) {
    default: return -1;
    case 't':
    switch (str[4]) {
    default: return -1;
    case 'S':
        if (strcmp(str + 5, "tatIndex") == 0)
            return name_LastStatIndex;
        break;
    case 'B':
        if (strcmp(str + 5, "uiltinName") == 0)
            return name_LastBuiltinName;
        break;
    }
    }
    case 'z':
        if (strcmp(str + 3, "y") == 0)
            return name_Lazy;
        break;
    }
    case 'o':
    switch (str[2]) {
    default: return -1;
    case 'o':
    switch (str[3]) {
    default: return -1;
    case 'p':
        if (strcmp(str + 4, "ProduceOutput") == 0)
            return name_LoopProduceOutput;
        break;
    case 'k':
    switch (str[4]) {
    default: return -1;
    case 'u':
    switch (str[5]) {
    default: return -1;
    case 'p':
    switch (str[6]) {
    default: return -1;
    case 'A':
        if (strcmp(str + 7, "ny") == 0)
            return name_LookupAny;
        break;
    case 'M':
        if (strcmp(str + 7, "odule") == 0)
            return name_LookupModule;
        break;
    case 'T':
        if (strcmp(str + 7, "ype") == 0)
            return name_LookupType;
        break;
    case 'F':
        if (strcmp(str + 7, "unction") == 0)
            return name_LookupFunction;
        break;
    }
    }
    }
    }
    }
    }
    case 'O':
    switch (str[1]) {
    default: return -1;
    case 'u':
    switch (str[2]) {
    default: return -1;
    case 't':
    switch (str[3]) {
    default: return -1;
    case 0:
        if (strcmp(str + 4, "") == 0)
            return name_Out;
        break;
    case 'p':
        if (strcmp(str + 4, "utsToList") == 0)
            return name_OutputsToList;
        break;
    }
    }
    }
    case 'N':
    switch (str[1]) {
    default: return -1;
    case 'a':
    switch (str[2]) {
    default: return -1;
    case 'm':
        if (strcmp(str + 3, "e") == 0)
            return name_Name;
        break;
    case 't':
        if (strcmp(str + 3, "ivePatch") == 0)
            return name_NativePatch;
        break;
    }
    case 'e':
        if (strcmp(str + 2, "wline") == 0)
            return name_Newline;
        break;
    case 'o':
    switch (str[2]) {
    default: return -1;
    case 't':
        if (strcmp(str + 3, "EnoughInputs") == 0)
            return name_NotEnoughInputs;
        break;
    case 'n':
        if (strcmp(str + 3, "e") == 0)
            return name_None;
        break;
    }
    }
    case 'P':
    switch (str[1]) {
    default: return -1;
    case 'a':
        if (strcmp(str + 2, "tchBlock") == 0)
            return name_PatchBlock;
        break;
    case 'r':
        if (strcmp(str + 2, "imary") == 0)
            return name_Primary;
        break;
    }
    case 'S':
    switch (str[1]) {
    default: return -1;
    case 'u':
        if (strcmp(str + 2, "ccess") == 0)
            return name_Success;
        break;
    case 't':
    switch (str[2]) {
    default: return -1;
    case 'r':
        if (strcmp(str + 3, "uctType") == 0)
            return name_StructType;
        break;
    case 'o':
    switch (str[3]) {
    default: return -1;
    case 'r':
    switch (str[4]) {
    default: return -1;
    case 'a':
    switch (str[5]) {
    default: return -1;
    case 'g':
    switch (str[6]) {
    default: return -1;
    case 'e':
    switch (str[7]) {
    default: return -1;
    case 'T':
    switch (str[8]) {
    default: return -1;
    case 'y':
    switch (str[9]) {
    default: return -1;
    case 'p':
    switch (str[10]) {
    default: return -1;
    case 'e':
    switch (str[11]) {
    default: return -1;
    case 'B':
        if (strcmp(str + 12, "ool") == 0)
            return name_StorageTypeBool;
        break;
    case 'F':
        if (strcmp(str + 12, "loat") == 0)
            return name_StorageTypeFloat;
        break;
    case 'I':
        if (strcmp(str + 12, "nt") == 0)
            return name_StorageTypeInt;
        break;
    case 'H':
    switch (str[12]) {
    default: return -1;
    case 'a':
    switch (str[13]) {
    default: return -1;
    case 's':
        if (strcmp(str + 14, "htable") == 0)
            return name_StorageTypeHashtable;
        break;
    case 'n':
        if (strcmp(str + 14, "dle") == 0)
            return name_StorageTypeHandle;
        break;
    }
    }
    case 'L':
        if (strcmp(str + 12, "ist") == 0)
            return name_StorageTypeList;
        break;
    case 'O':
    switch (str[12]) {
    default: return -1;
    case 'p':
        if (strcmp(str + 13, "aquePointer") == 0)
            return name_StorageTypeOpaquePointer;
        break;
    case 'b':
        if (strcmp(str + 13, "ject") == 0)
            return name_StorageTypeObject;
        break;
    }
    case 'N':
        if (strcmp(str + 12, "ull") == 0)
            return name_StorageTypeNull;
        break;
    case 'S':
        if (strcmp(str + 12, "tring") == 0)
            return name_StorageTypeString;
        break;
    case 'T':
    switch (str[12]) {
    default: return -1;
    case 'y':
        if (strcmp(str + 13, "pe") == 0)
            return name_StorageTypeType;
        break;
    case 'e':
        if (strcmp(str + 13, "rm") == 0)
            return name_StorageTypeTerm;
        break;
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    case 'R':
    switch (str[1]) {
    default: return -1;
    case 'e':
    switch (str[2]) {
    default: return -1;
    case 'p':
        if (strcmp(str + 3, "eat") == 0)
            return name_Repeat;
        break;
    case 't':
        if (strcmp(str + 3, "urn") == 0)
            return name_Return;
        break;
    }
    }
    case 'U':
    switch (str[1]) {
    default: return -1;
    case 'n':
    switch (str[2]) {
    default: return -1;
    case 'i':
        if (strcmp(str + 3, "formListType") == 0)
            return name_UniformListType;
        break;
    case 'k':
    switch (str[3]) {
    default: return -1;
    case 'n':
    switch (str[4]) {
    default: return -1;
    case 'o':
    switch (str[5]) {
    default: return -1;
    case 'w':
    switch (str[6]) {
    default: return -1;
    case 'n':
    switch (str[7]) {
    default: return -1;
    case 0:
        if (strcmp(str + 8, "") == 0)
            return name_Unknown;
        break;
    case 'I':
        if (strcmp(str + 8, "dentifier") == 0)
            return name_UnknownIdentifier;
        break;
    }
    }
    }
    }
    }
    case 'e':
        if (strcmp(str + 3, "valuated") == 0)
            return name_Unevaluated;
        break;
    case 'c':
        if (strcmp(str + 3, "aptured") == 0)
            return name_Uncaptured;
        break;
    case 't':
        if (strcmp(str + 3, "yped") == 0)
            return name_Untyped;
        break;
    }
    }
    case 'T':
    switch (str[1]) {
    default: return -1;
    case 'y':
        if (strcmp(str + 2, "peName") == 0)
            return name_TypeName;
        break;
    case 'e':
        if (strcmp(str + 2, "rmName") == 0)
            return name_TermName;
        break;
    case 'o':
        if (strcmp(str + 2, "oManyInputs") == 0)
            return name_TooManyInputs;
        break;
    }
    case 'W':
        if (strcmp(str + 1, "hitespace") == 0)
            return name_Whitespace;
        break;
    case 'o':
    switch (str[1]) {
    default: return -1;
    case 'p':
    switch (str[2]) {
    default: return -1;
    case '_':
    switch (str[3]) {
    default: return -1;
    case 'C':
    switch (str[4]) {
    default: return -1;
    case 'a':
    switch (str[5]) {
    default: return -1;
    case 's':
        if (strcmp(str + 6, "eBlock") == 0)
            return op_CaseBlock;
        break;
    case 'l':
        if (strcmp(str + 6, "lBlock") == 0)
            return op_CallBlock;
        break;
    }
    case 'l':
        if (strcmp(str + 5, "osureCall") == 0)
            return op_ClosureCall;
        break;
    case 'o':
        if (strcmp(str + 5, "ntinue") == 0)
            return op_Continue;
        break;
    }
    case 'B':
        if (strcmp(str + 4, "reak") == 0)
            return op_Break;
        break;
    case 'E':
    switch (str[4]) {
    default: return -1;
    case 'x':
        if (strcmp(str + 5, "itPoint") == 0)
            return op_ExitPoint;
        break;
    case 'r':
    switch (str[5]) {
    default: return -1;
    case 'r':
    switch (str[6]) {
    default: return -1;
    case 'o':
    switch (str[7]) {
    default: return -1;
    case 'r':
    switch (str[8]) {
    default: return -1;
    case 'T':
        if (strcmp(str + 9, "ooManyInputs") == 0)
            return op_ErrorTooManyInputs;
        break;
    case 'N':
        if (strcmp(str + 9, "otEnoughInputs") == 0)
            return op_ErrorNotEnoughInputs;
        break;
    }
    }
    }
    }
    }
    case 'D':
    switch (str[4]) {
    default: return -1;
    case 'y':
        if (strcmp(str + 5, "namicCall") == 0)
            return op_DynamicCall;
        break;
    case 'i':
        if (strcmp(str + 5, "scard") == 0)
            return op_Discard;
        break;
    }
    case 'F':
    switch (str[4]) {
    default: return -1;
    case 'i':
    switch (str[5]) {
    default: return -1;
    case 'r':
        if (strcmp(str + 6, "eNative") == 0)
            return op_FireNative;
        break;
    case 'n':
    switch (str[6]) {
    default: return -1;
    case 'i':
    switch (str[7]) {
    default: return -1;
    case 's':
    switch (str[8]) {
    default: return -1;
    case 'h':
    switch (str[9]) {
    default: return -1;
    case 'L':
        if (strcmp(str + 10, "oop") == 0)
            return op_FinishLoop;
        break;
    case 'F':
        if (strcmp(str + 10, "rame") == 0)
            return op_FinishFrame;
        break;
    }
    }
    }
    }
    }
    case 'o':
        if (strcmp(str + 5, "rLoop") == 0)
            return op_ForLoop;
        break;
    }
    case 'I':
        if (strcmp(str + 4, "nlineCopy") == 0)
            return op_InlineCopy;
        break;
    case 'N':
        if (strcmp(str + 4, "oOp") == 0)
            return op_NoOp;
        break;
    case 'P':
        if (strcmp(str + 4, "ause") == 0)
            return op_Pause;
        break;
    case 'S':
        if (strcmp(str + 4, "etNull") == 0)
            return op_SetNull;
        break;
    case 'R':
        if (strcmp(str + 4, "eturn") == 0)
            return op_Return;
        break;
    }
    }
    }
    case 's':
    switch (str[1]) {
    default: return -1;
    case 't':
    switch (str[2]) {
    default: return -1;
    case 'a':
    switch (str[3]) {
    default: return -1;
    case 't':
    switch (str[4]) {
    default: return -1;
    case '_':
    switch (str[5]) {
    default: return -1;
    case 'C':
    switch (str[6]) {
    default: return -1;
    case 'a':
    switch (str[7]) {
    default: return -1;
    case 's':
    switch (str[8]) {
    default: return -1;
    case 't':
    switch (str[9]) {
    default: return -1;
    case '_':
    switch (str[10]) {
    default: return -1;
    case 'P':
        if (strcmp(str + 11, "ushFrameWithInputs") == 0)
            return stat_Cast_PushFrameWithInputs;
        break;
    case 'L':
        if (strcmp(str + 11, "istCastElement") == 0)
            return stat_Cast_ListCastElement;
        break;
    case 'F':
        if (strcmp(str + 11, "inishFrame") == 0)
            return stat_Cast_FinishFrame;
        break;
    }
    }
    }
    }
    case 'o':
    switch (str[7]) {
    default: return -1;
    case 'p':
    switch (str[8]) {
    default: return -1;
    case 'y':
    switch (str[9]) {
    default: return -1;
    case '_':
    switch (str[10]) {
    default: return -1;
    case 'P':
    switch (str[11]) {
    default: return -1;
    case 'u':
    switch (str[12]) {
    default: return -1;
    case 's':
    switch (str[13]) {
    default: return -1;
    case 'h':
    switch (str[14]) {
    default: return -1;
    case 'e':
    switch (str[15]) {
    default: return -1;
    case 'd':
    switch (str[16]) {
    default: return -1;
    case 'I':
    switch (str[17]) {
    default: return -1;
    case 'n':
    switch (str[18]) {
    default: return -1;
    case 'p':
    switch (str[19]) {
    default: return -1;
    case 'u':
    switch (str[20]) {
    default: return -1;
    case 't':
    switch (str[21]) {
    default: return -1;
    case 'M':
        if (strcmp(str + 22, "ultiNewFrame") == 0)
            return stat_Copy_PushedInputMultiNewFrame;
        break;
    case 'N':
        if (strcmp(str + 22, "ewFrame") == 0)
            return stat_Copy_PushedInputNewFrame;
        break;
    }
    }
    }
    }
    }
    }
    }
    case 'F':
        if (strcmp(str + 15, "rameWithInputs") == 0)
            return stat_Copy_PushFrameWithInputs;
        break;
    }
    }
    }
    }
    case 'L':
    switch (str[11]) {
    default: return -1;
    case 'i':
        if (strcmp(str + 12, "stDuplicate") == 0)
            return stat_Copy_ListDuplicate;
        break;
    case 'o':
        if (strcmp(str + 12, "opCopyRebound") == 0)
            return stat_Copy_LoopCopyRebound;
        break;
    }
    }
    }
    }
    }
    }
    case 'B':
        if (strcmp(str + 6, "lockNameLookups") == 0)
            return stat_BlockNameLookups;
        break;
    case 'D':
    switch (str[6]) {
    default: return -1;
    case 'i':
        if (strcmp(str + 7, "ctHardCopy") == 0)
            return stat_DictHardCopy;
        break;
    case 'y':
    switch (str[7]) {
    default: return -1;
    case 'n':
    switch (str[8]) {
    default: return -1;
    case 'a':
    switch (str[9]) {
    default: return -1;
    case 'm':
    switch (str[10]) {
    default: return -1;
    case 'i':
    switch (str[11]) {
    default: return -1;
    case 'c':
    switch (str[12]) {
    default: return -1;
    case 'C':
        if (strcmp(str + 13, "all") == 0)
            return stat_DynamicCall;
        break;
    case 'M':
        if (strcmp(str + 13, "ethodCall") == 0)
            return stat_DynamicMethodCall;
        break;
    }
    }
    }
    }
    }
    }
    }
    case 'F':
        if (strcmp(str + 6, "inishDynamicCall") == 0)
            return stat_FinishDynamicCall;
        break;
    case 'I':
    switch (str[6]) {
    default: return -1;
    case 'n':
    switch (str[7]) {
    default: return -1;
    case 't':
    switch (str[8]) {
    default: return -1;
    case 'e':
    switch (str[9]) {
    default: return -1;
    case 'r':
    switch (str[10]) {
    default: return -1;
    case 'p':
        if (strcmp(str + 11, "reterCastOutputFromFinishedFrame") == 0)
            return stat_InterpreterCastOutputFromFinishedFrame;
        break;
    case 'n':
    switch (str[11]) {
    default: return -1;
    case 'e':
    switch (str[12]) {
    default: return -1;
    case 'd':
    switch (str[13]) {
    default: return -1;
    case 'N':
    switch (str[14]) {
    default: return -1;
    case 'a':
    switch (str[15]) {
    default: return -1;
    case 'm':
    switch (str[16]) {
    default: return -1;
    case 'e':
    switch (str[17]) {
    default: return -1;
    case 'C':
        if (strcmp(str + 18, "reate") == 0)
            return stat_InternedNameCreate;
        break;
    case 'L':
        if (strcmp(str + 18, "ookup") == 0)
            return stat_InternedNameLookup;
        break;
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    case 'L':
    switch (str[6]) {
    default: return -1;
    case 'i':
    switch (str[7]) {
    default: return -1;
    case 's':
    switch (str[8]) {
    default: return -1;
    case 't':
    switch (str[9]) {
    default: return -1;
    case 'H':
        if (strcmp(str + 10, "ardCopy") == 0)
            return stat_ListHardCopy;
        break;
    case 's':
    switch (str[10]) {
    default: return -1;
    case 'C':
        if (strcmp(str + 11, "reated") == 0)
            return stat_ListsCreated;
        break;
    case 'G':
        if (strcmp(str + 11, "rown") == 0)
            return stat_ListsGrown;
        break;
    }
    case 'S':
        if (strcmp(str + 10, "oftCopy") == 0)
            return stat_ListSoftCopy;
        break;
    }
    }
    }
    case 'o':
    switch (str[7]) {
    default: return -1;
    case 'o':
    switch (str[8]) {
    default: return -1;
    case 'p':
    switch (str[9]) {
    default: return -1;
    case 'W':
        if (strcmp(str + 10, "riteOutput") == 0)
            return stat_LoopWriteOutput;
        break;
    case 'F':
        if (strcmp(str + 10, "inishIteration") == 0)
            return stat_LoopFinishIteration;
        break;
    }
    }
    }
    }
    case 'P':
        if (strcmp(str + 6, "ushFrame") == 0)
            return stat_PushFrame;
        break;
    case 'S':
    switch (str[6]) {
    default: return -1;
    case 'e':
    switch (str[7]) {
    default: return -1;
    case 't':
    switch (str[8]) {
    default: return -1;
    case 'I':
        if (strcmp(str + 9, "ndex") == 0)
            return stat_SetIndex;
        break;
    case 'F':
        if (strcmp(str + 9, "ield") == 0)
            return stat_SetField;
        break;
    }
    }
    case 't':
    switch (str[7]) {
    default: return -1;
    case 'r':
    switch (str[8]) {
    default: return -1;
    case 'i':
    switch (str[9]) {
    default: return -1;
    case 'n':
    switch (str[10]) {
    default: return -1;
    case 'g':
    switch (str[11]) {
    default: return -1;
    case 'S':
        if (strcmp(str + 12, "oftCopy") == 0)
            return stat_StringSoftCopy;
        break;
    case 'R':
    switch (str[12]) {
    default: return -1;
    case 'e':
    switch (str[13]) {
    default: return -1;
    case 's':
    switch (str[14]) {
    default: return -1;
    case 'i':
    switch (str[15]) {
    default: return -1;
    case 'z':
    switch (str[16]) {
    default: return -1;
    case 'e':
    switch (str[17]) {
    default: return -1;
    case 'I':
        if (strcmp(str + 18, "nPlace") == 0)
            return stat_StringResizeInPlace;
        break;
    case 'C':
        if (strcmp(str + 18, "reate") == 0)
            return stat_StringResizeCreate;
        break;
    }
    }
    }
    }
    }
    }
    case 'C':
        if (strcmp(str + 12, "reate") == 0)
            return stat_StringCreate;
        break;
    case 'T':
        if (strcmp(str + 12, "oStd") == 0)
            return stat_StringToStd;
        break;
    case 'D':
        if (strcmp(str + 12, "uplicate") == 0)
            return stat_StringDuplicate;
        break;
    }
    }
    }
    }
    case 'e':
        if (strcmp(str + 8, "pInterpreter") == 0)
            return stat_StepInterpreter;
        break;
    }
    }
    case 'T':
    switch (str[6]) {
    default: return -1;
    case 'e':
    switch (str[7]) {
    default: return -1;
    case 'r':
    switch (str[8]) {
    default: return -1;
    case 'm':
    switch (str[9]) {
    default: return -1;
    case 'P':
    switch (str[10]) {
    default: return -1;
    case 'r':
    switch (str[11]) {
    default: return -1;
    case 'o':
    switch (str[12]) {
    default: return -1;
    case 'p':
    switch (str[13]) {
    default: return -1;
    case 'A':
    switch (str[14]) {
    default: return -1;
    case 'c':
        if (strcmp(str + 15, "cess") == 0)
            return stat_TermPropAccess;
        break;
    case 'd':
        if (strcmp(str + 15, "ded") == 0)
            return stat_TermPropAdded;
        break;
    }
    }
    }
    }
    }
    case 's':
        if (strcmp(str + 10, "Created") == 0)
            return stat_TermsCreated;
        break;
    }
    }
    }
    case 'o':
        if (strcmp(str + 7, "uch_ListCast") == 0)
            return stat_Touch_ListCast;
        break;
    }
    case 'W':
        if (strcmp(str + 6, "riteTermBytecode") == 0)
            return stat_WriteTermBytecode;
        break;
    case 'V':
    switch (str[6]) {
    default: return -1;
    case 'a':
    switch (str[7]) {
    default: return -1;
    case 'l':
    switch (str[8]) {
    default: return -1;
    case 'u':
    switch (str[9]) {
    default: return -1;
    case 'e':
    switch (str[10]) {
    default: return -1;
    case 'C':
    switch (str[11]) {
    default: return -1;
    case 'a':
    switch (str[12]) {
    default: return -1;
    case 's':
    switch (str[13]) {
    default: return -1;
    case 't':
    switch (str[14]) {
    default: return -1;
    case 0:
        if (strcmp(str + 15, "") == 0)
            return stat_ValueCast;
        break;
    case 'D':
        if (strcmp(str + 15, "ispatched") == 0)
            return stat_ValueCastDispatched;
        break;
    }
    }
    }
    case 'r':
        if (strcmp(str + 12, "eates") == 0)
            return stat_ValueCreates;
        break;
    case 'o':
        if (strcmp(str + 12, "pies") == 0)
            return stat_ValueCopies;
        break;
    }
    case 'T':
        if (strcmp(str + 11, "ouch") == 0)
            return stat_ValueTouch;
        break;
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    case 't':
    switch (str[1]) {
    default: return -1;
    case 'o':
    switch (str[2]) {
    default: return -1;
    case 'k':
    switch (str[3]) {
    default: return -1;
    case '_':
    switch (str[4]) {
    default: return -1;
    case 'A':
    switch (str[5]) {
    default: return -1;
    case 'm':
        if (strcmp(str + 6, "persand") == 0)
            return tok_Ampersand;
        break;
    case 't':
        if (strcmp(str + 6, "") == 0)
            return tok_At;
        break;
    case 'n':
        if (strcmp(str + 6, "d") == 0)
            return tok_And;
        break;
    }
    case 'C':
    switch (str[5]) {
    default: return -1;
    case 'a':
        if (strcmp(str + 6, "se") == 0)
            return tok_Case;
        break;
    case 'o':
    switch (str[6]) {
    default: return -1;
    case 'm':
    switch (str[7]) {
    default: return -1;
    case 'm':
    switch (str[8]) {
    default: return -1;
    case 'a':
        if (strcmp(str + 9, "") == 0)
            return tok_Comma;
        break;
    case 'e':
        if (strcmp(str + 9, "nt") == 0)
            return tok_Comment;
        break;
    }
    }
    case 'l':
    switch (str[7]) {
    default: return -1;
    case 'o':
    switch (str[8]) {
    default: return -1;
    case 'r':
        if (strcmp(str + 9, "") == 0)
            return tok_Color;
        break;
    case 'n':
    switch (str[9]) {
    default: return -1;
    case 0:
        if (strcmp(str + 10, "") == 0)
            return tok_Colon;
        break;
    case 'S':
        if (strcmp(str + 10, "tring") == 0)
            return tok_ColonString;
        break;
    case 'E':
        if (strcmp(str + 10, "quals") == 0)
            return tok_ColonEquals;
        break;
    }
    }
    }
    case 'n':
        if (strcmp(str + 7, "tinue") == 0)
            return tok_Continue;
        break;
    }
    }
    case 'B':
    switch (str[5]) {
    default: return -1;
    case 'r':
        if (strcmp(str + 6, "eak") == 0)
            return tok_Break;
        break;
    case 'o':
        if (strcmp(str + 6, "ol") == 0)
            return tok_Bool;
        break;
    }
    case 'E':
    switch (str[5]) {
    default: return -1;
    case 'q':
        if (strcmp(str + 6, "uals") == 0)
            return tok_Equals;
        break;
    case 'l':
    switch (str[6]) {
    default: return -1;
    case 'i':
        if (strcmp(str + 7, "f") == 0)
            return tok_Elif;
        break;
    case 's':
        if (strcmp(str + 7, "e") == 0)
            return tok_Else;
        break;
    case 'l':
        if (strcmp(str + 7, "ipsis") == 0)
            return tok_Ellipsis;
        break;
    }
    case 'o':
        if (strcmp(str + 6, "f") == 0)
            return tok_Eof;
        break;
    }
    case 'D':
    switch (str[5]) {
    default: return -1;
    case 'i':
        if (strcmp(str + 6, "scard") == 0)
            return tok_Discard;
        break;
    case 'e':
        if (strcmp(str + 6, "f") == 0)
            return tok_Def;
        break;
    case 'o':
    switch (str[6]) {
    default: return -1;
    case 'u':
    switch (str[7]) {
    default: return -1;
    case 'b':
    switch (str[8]) {
    default: return -1;
    case 'l':
    switch (str[9]) {
    default: return -1;
    case 'e':
    switch (str[10]) {
    default: return -1;
    case 'A':
        if (strcmp(str + 11, "mpersand") == 0)
            return tok_DoubleAmpersand;
        break;
    case 'S':
        if (strcmp(str + 11, "lash") == 0)
            return tok_DoubleSlash;
        break;
    case 'E':
        if (strcmp(str + 11, "quals") == 0)
            return tok_DoubleEquals;
        break;
    case 'C':
        if (strcmp(str + 11, "olon") == 0)
            return tok_DoubleColon;
        break;
    case 'V':
        if (strcmp(str + 11, "erticalBar") == 0)
            return tok_DoubleVerticalBar;
        break;
    }
    }
    }
    }
    case 't':
    switch (str[7]) {
    default: return -1;
    case 0:
        if (strcmp(str + 8, "") == 0)
            return tok_Dot;
        break;
    case 'A':
        if (strcmp(str + 8, "t") == 0)
            return tok_DotAt;
        break;
    }
    }
    }
    case 'G':
    switch (str[5]) {
    default: return -1;
    case 'T':
    switch (str[6]) {
    default: return -1;
    case 'h':
    switch (str[7]) {
    default: return -1;
    case 'a':
    switch (str[8]) {
    default: return -1;
    case 'n':
    switch (str[9]) {
    default: return -1;
    case 0:
        if (strcmp(str + 10, "") == 0)
            return tok_GThan;
        break;
    case 'E':
        if (strcmp(str + 10, "q") == 0)
            return tok_GThanEq;
        break;
    }
    }
    }
    }
    }
    case 'F':
    switch (str[5]) {
    default: return -1;
    case 'a':
        if (strcmp(str + 6, "lse") == 0)
            return tok_False;
        break;
    case 'l':
        if (strcmp(str + 6, "oat") == 0)
            return tok_Float;
        break;
    case 'o':
        if (strcmp(str + 6, "r") == 0)
            return tok_For;
        break;
    }
    case 'I':
    switch (str[5]) {
    default: return -1;
    case 'n':
    switch (str[6]) {
    default: return -1;
    case 0:
        if (strcmp(str + 7, "") == 0)
            return tok_In;
        break;
    case 'c':
        if (strcmp(str + 7, "lude") == 0)
            return tok_Include;
        break;
    case 't':
        if (strcmp(str + 7, "eger") == 0)
            return tok_Integer;
        break;
    }
    case 'd':
        if (strcmp(str + 6, "entifier") == 0)
            return tok_Identifier;
        break;
    case 'f':
        if (strcmp(str + 6, "") == 0)
            return tok_If;
        break;
    }
    case 'H':
        if (strcmp(str + 5, "exInteger") == 0)
            return tok_HexInteger;
        break;
    case 'M':
    switch (str[5]) {
    default: return -1;
    case 'i':
    switch (str[6]) {
    default: return -1;
    case 'n':
    switch (str[7]) {
    default: return -1;
    case 'u':
    switch (str[8]) {
    default: return -1;
    case 's':
    switch (str[9]) {
    default: return -1;
    case 0:
        if (strcmp(str + 10, "") == 0)
            return tok_Minus;
        break;
    case 'E':
        if (strcmp(str + 10, "quals") == 0)
            return tok_MinusEquals;
        break;
    }
    }
    }
    }
    }
    case 'L':
    switch (str[5]) {
    default: return -1;
    case 'P':
        if (strcmp(str + 6, "aren") == 0)
            return tok_LParen;
        break;
    case 'B':
    switch (str[6]) {
    default: return -1;
    case 'r':
    switch (str[7]) {
    default: return -1;
    case 'a':
    switch (str[8]) {
    default: return -1;
    case 'c':
    switch (str[9]) {
    default: return -1;
    case 'k':
        if (strcmp(str + 10, "et") == 0)
            return tok_LBracket;
        break;
    case 'e':
        if (strcmp(str + 10, "") == 0)
            return tok_LBrace;
        break;
    }
    }
    }
    }
    case 'e':
        if (strcmp(str + 6, "ftArrow") == 0)
            return tok_LeftArrow;
        break;
    case 'T':
    switch (str[6]) {
    default: return -1;
    case 'h':
    switch (str[7]) {
    default: return -1;
    case 'a':
    switch (str[8]) {
    default: return -1;
    case 'n':
    switch (str[9]) {
    default: return -1;
    case 0:
        if (strcmp(str + 10, "") == 0)
            return tok_LThan;
        break;
    case 'E':
        if (strcmp(str + 10, "q") == 0)
            return tok_LThanEq;
        break;
    }
    }
    }
    }
    }
    case 'O':
        if (strcmp(str + 5, "r") == 0)
            return tok_Or;
        break;
    case 'N':
    switch (str[5]) {
    default: return -1;
    case 'a':
        if (strcmp(str + 6, "mespace") == 0)
            return tok_Namespace;
        break;
    case 'u':
        if (strcmp(str + 6, "ll") == 0)
            return tok_Null;
        break;
    case 'e':
        if (strcmp(str + 6, "wline") == 0)
            return tok_Newline;
        break;
    case 'o':
    switch (str[6]) {
    default: return -1;
    case 't':
    switch (str[7]) {
    default: return -1;
    case 0:
        if (strcmp(str + 8, "") == 0)
            return tok_Not;
        break;
    case 'E':
        if (strcmp(str + 8, "quals") == 0)
            return tok_NotEquals;
        break;
    }
    }
    }
    case 'Q':
        if (strcmp(str + 5, "uestion") == 0)
            return tok_Question;
        break;
    case 'P':
    switch (str[5]) {
    default: return -1;
    case 'a':
        if (strcmp(str + 6, "ckage") == 0)
            return tok_Package;
        break;
    case 'e':
        if (strcmp(str + 6, "rcent") == 0)
            return tok_Percent;
        break;
    case 'l':
    switch (str[6]) {
    default: return -1;
    case 'u':
    switch (str[7]) {
    default: return -1;
    case 's':
    switch (str[8]) {
    default: return -1;
    case 0:
        if (strcmp(str + 9, "") == 0)
            return tok_Plus;
        break;
    case 'E':
        if (strcmp(str + 9, "quals") == 0)
            return tok_PlusEquals;
        break;
    }
    }
    }
    case 'o':
        if (strcmp(str + 6, "und") == 0)
            return tok_Pound;
        break;
    }
    case 'S':
    switch (str[5]) {
    default: return -1;
    case 'e':
    switch (str[6]) {
    default: return -1;
    case 'c':
        if (strcmp(str + 7, "tion") == 0)
            return tok_Section;
        break;
    case 'm':
        if (strcmp(str + 7, "icolon") == 0)
            return tok_Semicolon;
        break;
    }
    case 't':
    switch (str[6]) {
    default: return -1;
    case 'a':
    switch (str[7]) {
    default: return -1;
    case 'r':
    switch (str[8]) {
    default: return -1;
    case 0:
        if (strcmp(str + 9, "") == 0)
            return tok_Star;
        break;
    case 'E':
        if (strcmp(str + 9, "quals") == 0)
            return tok_StarEquals;
        break;
    }
    case 't':
        if (strcmp(str + 8, "e") == 0)
            return tok_State;
        break;
    }
    case 'r':
        if (strcmp(str + 7, "ing") == 0)
            return tok_String;
        break;
    }
    case 'w':
        if (strcmp(str + 6, "itch") == 0)
            return tok_Switch;
        break;
    case 'l':
    switch (str[6]) {
    default: return -1;
    case 'a':
    switch (str[7]) {
    default: return -1;
    case 's':
    switch (str[8]) {
    default: return -1;
    case 'h':
    switch (str[9]) {
    default: return -1;
    case 0:
        if (strcmp(str + 10, "") == 0)
            return tok_Slash;
        break;
    case 'E':
        if (strcmp(str + 10, "quals") == 0)
            return tok_SlashEquals;
        break;
    }
    }
    }
    }
    }
    case 'R':
    switch (str[5]) {
    default: return -1;
    case 'i':
        if (strcmp(str + 6, "ghtArrow") == 0)
            return tok_RightArrow;
        break;
    case 'P':
        if (strcmp(str + 6, "aren") == 0)
            return tok_RParen;
        break;
    case 'B':
    switch (str[6]) {
    default: return -1;
    case 'r':
    switch (str[7]) {
    default: return -1;
    case 'a':
    switch (str[8]) {
    default: return -1;
    case 'c':
    switch (str[9]) {
    default: return -1;
    case 'k':
        if (strcmp(str + 10, "et") == 0)
            return tok_RBracket;
        break;
    case 'e':
        if (strcmp(str + 10, "") == 0)
            return tok_RBrace;
        break;
    }
    }
    }
    }
    case 'e':
    switch (str[6]) {
    default: return -1;
    case 'q':
        if (strcmp(str + 7, "uire") == 0)
            return tok_Require;
        break;
    case 't':
        if (strcmp(str + 7, "urn") == 0)
            return tok_Return;
        break;
    }
    }
    case 'U':
    switch (str[5]) {
    default: return -1;
    case 'n':
    switch (str[6]) {
    default: return -1;
    case 'r':
        if (strcmp(str + 7, "ecognized") == 0)
            return tok_Unrecognized;
        break;
    case 'u':
    switch (str[7]) {
    default: return -1;
    case 's':
    switch (str[8]) {
    default: return -1;
    case 'e':
    switch (str[9]) {
    default: return -1;
    case 'd':
    switch (str[10]) {
    default: return -1;
    case 'N':
    switch (str[11]) {
    default: return -1;
    case 'a':
    switch (str[12]) {
    default: return -1;
    case 'm':
    switch (str[13]) {
    default: return -1;
    case 'e':
    switch (str[14]) {
    default: return -1;
    case '1':
        if (strcmp(str + 15, "") == 0)
            return tok_UnusedName1;
        break;
    case '3':
        if (strcmp(str + 15, "") == 0)
            return tok_UnusedName3;
        break;
    case '2':
        if (strcmp(str + 15, "") == 0)
            return tok_UnusedName2;
        break;
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    case 'T':
    switch (str[5]) {
    default: return -1;
    case 'y':
        if (strcmp(str + 6, "pe") == 0)
            return tok_Type;
        break;
    case 'r':
    switch (str[6]) {
    default: return -1;
    case 'i':
    switch (str[7]) {
    default: return -1;
    case 'p':
    switch (str[8]) {
    default: return -1;
    case 'l':
    switch (str[9]) {
    default: return -1;
    case 'e':
    switch (str[10]) {
    default: return -1;
    case 'L':
        if (strcmp(str + 11, "Than") == 0)
            return tok_TripleLThan;
        break;
    case 'G':
        if (strcmp(str + 11, "Than") == 0)
            return tok_TripleGThan;
        break;
    }
    }
    }
    }
    case 'u':
        if (strcmp(str + 7, "e") == 0)
            return tok_True;
        break;
    }
    case 'w':
        if (strcmp(str + 6, "oDots") == 0)
            return tok_TwoDots;
        break;
    }
    case 'W':
    switch (str[5]) {
    default: return -1;
    case 'h':
    switch (str[6]) {
    default: return -1;
    case 'i':
    switch (str[7]) {
    default: return -1;
    case 'l':
        if (strcmp(str + 8, "e") == 0)
            return tok_While;
        break;
    case 't':
        if (strcmp(str + 8, "espace") == 0)
            return tok_Whitespace;
        break;
    }
    }
    }
    }
    }
    }
    }
    }

    return -1;
}
} // namespace circa
