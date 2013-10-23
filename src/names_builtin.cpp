// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#include "common_headers.h"
#include "names_builtin.h"

namespace circa {

const char* builtin_symbol_to_string(int name)
{
    if (name >= sym_LastBuiltinName)
        return NULL;

    switch (name) {
    case sym_None: return "None";
    case sym_Invalid: return "Invalid";
    case sym_File: return "File";
    case sym_Newline: return "Newline";
    case sym_Out: return "Out";
    case sym_Unknown: return "Unknown";
    case sym_Repeat: return "Repeat";
    case sym_Success: return "Success";
    case sym_Failure: return "Failure";
    case sym_Yes: return "Yes";
    case sym_No: return "No";
    case sym_Maybe: return "Maybe";
    case sym_Index: return "Index";
    case sym_Last: return "Last";
    case sym_EvaluationEmpty: return "EvaluationEmpty";
    case sym_HasEffects: return "HasEffects";
    case sym_HasControlFlow: return "HasControlFlow";
    case sym_DirtyStateType: return "DirtyStateType";
    case sym_Filename: return "Filename";
    case sym_Builtins: return "Builtins";
    case sym_Wildcard: return "Wildcard";
    case sym_RecursiveWildcard: return "RecursiveWildcard";
    case sym_Function: return "Function";
    case sym_FileNotFound: return "FileNotFound";
    case sym_NotEnoughInputs: return "NotEnoughInputs";
    case sym_TooManyInputs: return "TooManyInputs";
    case sym_ExtraOutputNotFound: return "ExtraOutputNotFound";
    case sym_Default: return "Default";
    case sym_ByDemand: return "ByDemand";
    case sym_Unevaluated: return "Unevaluated";
    case sym_InProgress: return "InProgress";
    case sym_Lazy: return "Lazy";
    case sym_Consumed: return "Consumed";
    case sym_Uncaptured: return "Uncaptured";
    case sym_Return: return "Return";
    case sym_Continue: return "Continue";
    case sym_Break: return "Break";
    case sym_Discard: return "Discard";
    case sym_Control: return "Control";
    case sym_ExitLevelFunction: return "ExitLevelFunction";
    case sym_ExitLevelLoop: return "ExitLevelLoop";
    case sym_HighestExitLevel: return "HighestExitLevel";
    case sym_ExtraReturn: return "ExtraReturn";
    case sym_Name: return "Name";
    case sym_Primary: return "Primary";
    case sym_Anonymous: return "Anonymous";
    case sym_State: return "State";
    case sym_StackReady: return "StackReady";
    case sym_StackRunning: return "StackRunning";
    case sym_StackFinished: return "StackFinished";
    case sym_InfixOperator: return "InfixOperator";
    case sym_FunctionName: return "FunctionName";
    case sym_TypeName: return "TypeName";
    case sym_TermName: return "TermName";
    case sym_Keyword: return "Keyword";
    case sym_Whitespace: return "Whitespace";
    case sym_UnknownIdentifier: return "UnknownIdentifier";
    case sym_LookupAny: return "LookupAny";
    case sym_LookupType: return "LookupType";
    case sym_LookupFunction: return "LookupFunction";
    case sym_LookupModule: return "LookupModule";
    case sym_Untyped: return "Untyped";
    case sym_UniformListType: return "UniformListType";
    case sym_AnonStructType: return "AnonStructType";
    case sym_StructType: return "StructType";
    case sym_NativePatch: return "NativePatch";
    case sym_PatchBlock: return "PatchBlock";
    case sym_Filesystem: return "Filesystem";
    case sym_Bootstrapping: return "Bootstrapping";
    case sym_Done: return "Done";
    case sym_StorageTypeNull: return "StorageTypeNull";
    case sym_StorageTypeInt: return "StorageTypeInt";
    case sym_StorageTypeFloat: return "StorageTypeFloat";
    case sym_StorageTypeBool: return "StorageTypeBool";
    case sym_StorageTypeStack: return "StorageTypeStack";
    case sym_StorageTypeString: return "StorageTypeString";
    case sym_StorageTypeList: return "StorageTypeList";
    case sym_StorageTypeOpaquePointer: return "StorageTypeOpaquePointer";
    case sym_StorageTypeTerm: return "StorageTypeTerm";
    case sym_StorageTypeType: return "StorageTypeType";
    case sym_StorageTypeHandle: return "StorageTypeHandle";
    case sym_StorageTypeHashtable: return "StorageTypeHashtable";
    case sym_StorageTypeObject: return "StorageTypeObject";
    case sym_ChangeAppend: return "ChangeAppend";
    case sym_ChangeRename: return "ChangeRename";
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
    case tok_DoubleStar: return "tok_DoubleStar";
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
    case tok_While: return "tok_While";
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
    case tok_Require: return "tok_Require";
    case tok_Package: return "tok_Package";
    case tok_Section: return "tok_Section";
    case tok_Whitespace: return "tok_Whitespace";
    case tok_Newline: return "tok_Newline";
    case tok_Comment: return "tok_Comment";
    case tok_Eof: return "tok_Eof";
    case tok_Unrecognized: return "tok_Unrecognized";
    case sym_NormalCall: return "NormalCall";
    case sym_FuncApply: return "FuncApply";
    case sym_FuncCall: return "FuncCall";
    case sym_FirstStatIndex: return "FirstStatIndex";
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
    case sym_LastStatIndex: return "LastStatIndex";
    case sym_LastBuiltinName: return "LastBuiltinName";
    default: return NULL;
    }
}

int builtin_symbol_from_string(const char* str)
{
    switch (str[0]) {
    case 'A':
    switch (str[1]) {
    case 'n':
    switch (str[2]) {
    case 'o':
    switch (str[3]) {
    case 'n':
    switch (str[4]) {
    case 'S':
        if (strcmp(str + 5, "tructType") == 0)
            return sym_AnonStructType;
        break;
    case 'y':
        if (strcmp(str + 5, "mous") == 0)
            return sym_Anonymous;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'B':
    switch (str[1]) {
    case 'o':
        if (strcmp(str + 2, "otstrapping") == 0)
            return sym_Bootstrapping;
        break;
    case 'r':
        if (strcmp(str + 2, "eak") == 0)
            return sym_Break;
        break;
    case 'u':
        if (strcmp(str + 2, "iltins") == 0)
            return sym_Builtins;
        break;
    case 'y':
        if (strcmp(str + 2, "Demand") == 0)
            return sym_ByDemand;
        break;
    default: return -1;
    }
    case 'C':
    switch (str[1]) {
    case 'h':
    switch (str[2]) {
    case 'a':
    switch (str[3]) {
    case 'n':
    switch (str[4]) {
    case 'g':
    switch (str[5]) {
    case 'e':
    switch (str[6]) {
    case 'A':
        if (strcmp(str + 7, "ppend") == 0)
            return sym_ChangeAppend;
        break;
    case 'R':
        if (strcmp(str + 7, "ename") == 0)
            return sym_ChangeRename;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
    switch (str[2]) {
    case 'n':
    switch (str[3]) {
    case 's':
        if (strcmp(str + 4, "umed") == 0)
            return sym_Consumed;
        break;
    case 't':
    switch (str[4]) {
    case 'i':
        if (strcmp(str + 5, "nue") == 0)
            return sym_Continue;
        break;
    case 'r':
        if (strcmp(str + 5, "ol") == 0)
            return sym_Control;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'D':
    switch (str[1]) {
    case 'e':
        if (strcmp(str + 2, "fault") == 0)
            return sym_Default;
        break;
    case 'i':
    switch (str[2]) {
    case 'r':
        if (strcmp(str + 3, "tyStateType") == 0)
            return sym_DirtyStateType;
        break;
    case 's':
        if (strcmp(str + 3, "card") == 0)
            return sym_Discard;
        break;
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 2, "ne") == 0)
            return sym_Done;
        break;
    default: return -1;
    }
    case 'E':
    switch (str[1]) {
    case 'v':
        if (strcmp(str + 2, "aluationEmpty") == 0)
            return sym_EvaluationEmpty;
        break;
    case 'x':
    switch (str[2]) {
    case 'i':
    switch (str[3]) {
    case 't':
    switch (str[4]) {
    case 'L':
    switch (str[5]) {
    case 'e':
    switch (str[6]) {
    case 'v':
    switch (str[7]) {
    case 'e':
    switch (str[8]) {
    case 'l':
    switch (str[9]) {
    case 'F':
        if (strcmp(str + 10, "unction") == 0)
            return sym_ExitLevelFunction;
        break;
    case 'L':
        if (strcmp(str + 10, "oop") == 0)
            return sym_ExitLevelLoop;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 't':
    switch (str[3]) {
    case 'r':
    switch (str[4]) {
    case 'a':
    switch (str[5]) {
    case 'O':
        if (strcmp(str + 6, "utputNotFound") == 0)
            return sym_ExtraOutputNotFound;
        break;
    case 'R':
        if (strcmp(str + 6, "eturn") == 0)
            return sym_ExtraReturn;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'F':
    switch (str[1]) {
    case 'a':
        if (strcmp(str + 2, "ilure") == 0)
            return sym_Failure;
        break;
    case 'i':
    switch (str[2]) {
    case 'l':
    switch (str[3]) {
    case 'e':
    switch (str[4]) {
    case 'N':
        if (strcmp(str + 5, "otFound") == 0)
            return sym_FileNotFound;
        break;
    case 'n':
        if (strcmp(str + 5, "ame") == 0)
            return sym_Filename;
        break;
    case 's':
        if (strcmp(str + 5, "ystem") == 0)
            return sym_Filesystem;
        break;
    case 0:
            return sym_File;
    default: return -1;
    }
    default: return -1;
    }
    case 'r':
        if (strcmp(str + 3, "stStatIndex") == 0)
            return sym_FirstStatIndex;
        break;
    default: return -1;
    }
    case 'u':
    switch (str[2]) {
    case 'n':
    switch (str[3]) {
    case 'c':
    switch (str[4]) {
    case 'A':
        if (strcmp(str + 5, "pply") == 0)
            return sym_FuncApply;
        break;
    case 'C':
        if (strcmp(str + 5, "all") == 0)
            return sym_FuncCall;
        break;
    case 't':
    switch (str[5]) {
    case 'i':
    switch (str[6]) {
    case 'o':
    switch (str[7]) {
    case 'n':
    switch (str[8]) {
    case 'N':
        if (strcmp(str + 9, "ame") == 0)
            return sym_FunctionName;
        break;
    case 0:
            return sym_Function;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'H':
    switch (str[1]) {
    case 'a':
    switch (str[2]) {
    case 's':
    switch (str[3]) {
    case 'C':
        if (strcmp(str + 4, "ontrolFlow") == 0)
            return sym_HasControlFlow;
        break;
    case 'E':
        if (strcmp(str + 4, "ffects") == 0)
            return sym_HasEffects;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'i':
        if (strcmp(str + 2, "ghestExitLevel") == 0)
            return sym_HighestExitLevel;
        break;
    default: return -1;
    }
    case 'I':
    switch (str[1]) {
    case 'n':
    switch (str[2]) {
    case 'P':
        if (strcmp(str + 3, "rogress") == 0)
            return sym_InProgress;
        break;
    case 'd':
        if (strcmp(str + 3, "ex") == 0)
            return sym_Index;
        break;
    case 'f':
        if (strcmp(str + 3, "ixOperator") == 0)
            return sym_InfixOperator;
        break;
    case 'v':
        if (strcmp(str + 3, "alid") == 0)
            return sym_Invalid;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'K':
        if (strcmp(str + 1, "eyword") == 0)
            return sym_Keyword;
        break;
    case 'L':
    switch (str[1]) {
    case 'a':
    switch (str[2]) {
    case 's':
    switch (str[3]) {
    case 't':
    switch (str[4]) {
    case 'B':
        if (strcmp(str + 5, "uiltinName") == 0)
            return sym_LastBuiltinName;
        break;
    case 'S':
        if (strcmp(str + 5, "tatIndex") == 0)
            return sym_LastStatIndex;
        break;
    case 0:
            return sym_Last;
    default: return -1;
    }
    default: return -1;
    }
    case 'z':
        if (strcmp(str + 3, "y") == 0)
            return sym_Lazy;
        break;
    default: return -1;
    }
    case 'o':
    switch (str[2]) {
    case 'o':
    switch (str[3]) {
    case 'k':
    switch (str[4]) {
    case 'u':
    switch (str[5]) {
    case 'p':
    switch (str[6]) {
    case 'A':
        if (strcmp(str + 7, "ny") == 0)
            return sym_LookupAny;
        break;
    case 'F':
        if (strcmp(str + 7, "unction") == 0)
            return sym_LookupFunction;
        break;
    case 'M':
        if (strcmp(str + 7, "odule") == 0)
            return sym_LookupModule;
        break;
    case 'T':
        if (strcmp(str + 7, "ype") == 0)
            return sym_LookupType;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'M':
        if (strcmp(str + 1, "aybe") == 0)
            return sym_Maybe;
        break;
    case 'N':
    switch (str[1]) {
    case 'a':
    switch (str[2]) {
    case 'm':
        if (strcmp(str + 3, "e") == 0)
            return sym_Name;
        break;
    case 't':
        if (strcmp(str + 3, "ivePatch") == 0)
            return sym_NativePatch;
        break;
    default: return -1;
    }
    case 'e':
        if (strcmp(str + 2, "wline") == 0)
            return sym_Newline;
        break;
    case 'o':
    switch (str[2]) {
    case 'n':
        if (strcmp(str + 3, "e") == 0)
            return sym_None;
        break;
    case 'r':
        if (strcmp(str + 3, "malCall") == 0)
            return sym_NormalCall;
        break;
    case 't':
        if (strcmp(str + 3, "EnoughInputs") == 0)
            return sym_NotEnoughInputs;
        break;
    case 0:
            return sym_No;
    default: return -1;
    }
    default: return -1;
    }
    case 'O':
        if (strcmp(str + 1, "ut") == 0)
            return sym_Out;
        break;
    case 'P':
    switch (str[1]) {
    case 'a':
        if (strcmp(str + 2, "tchBlock") == 0)
            return sym_PatchBlock;
        break;
    case 'r':
        if (strcmp(str + 2, "imary") == 0)
            return sym_Primary;
        break;
    default: return -1;
    }
    case 'R':
    switch (str[1]) {
    case 'e':
    switch (str[2]) {
    case 'c':
        if (strcmp(str + 3, "ursiveWildcard") == 0)
            return sym_RecursiveWildcard;
        break;
    case 'p':
        if (strcmp(str + 3, "eat") == 0)
            return sym_Repeat;
        break;
    case 't':
        if (strcmp(str + 3, "urn") == 0)
            return sym_Return;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'S':
    switch (str[1]) {
    case 't':
    switch (str[2]) {
    case 'a':
    switch (str[3]) {
    case 'c':
    switch (str[4]) {
    case 'k':
    switch (str[5]) {
    case 'F':
        if (strcmp(str + 6, "inished") == 0)
            return sym_StackFinished;
        break;
    case 'R':
    switch (str[6]) {
    case 'e':
        if (strcmp(str + 7, "ady") == 0)
            return sym_StackReady;
        break;
    case 'u':
        if (strcmp(str + 7, "nning") == 0)
            return sym_StackRunning;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 't':
        if (strcmp(str + 4, "e") == 0)
            return sym_State;
        break;
    default: return -1;
    }
    case 'o':
    switch (str[3]) {
    case 'r':
    switch (str[4]) {
    case 'a':
    switch (str[5]) {
    case 'g':
    switch (str[6]) {
    case 'e':
    switch (str[7]) {
    case 'T':
    switch (str[8]) {
    case 'y':
    switch (str[9]) {
    case 'p':
    switch (str[10]) {
    case 'e':
    switch (str[11]) {
    case 'B':
        if (strcmp(str + 12, "ool") == 0)
            return sym_StorageTypeBool;
        break;
    case 'F':
        if (strcmp(str + 12, "loat") == 0)
            return sym_StorageTypeFloat;
        break;
    case 'H':
    switch (str[12]) {
    case 'a':
    switch (str[13]) {
    case 'n':
        if (strcmp(str + 14, "dle") == 0)
            return sym_StorageTypeHandle;
        break;
    case 's':
        if (strcmp(str + 14, "htable") == 0)
            return sym_StorageTypeHashtable;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'I':
        if (strcmp(str + 12, "nt") == 0)
            return sym_StorageTypeInt;
        break;
    case 'L':
        if (strcmp(str + 12, "ist") == 0)
            return sym_StorageTypeList;
        break;
    case 'N':
        if (strcmp(str + 12, "ull") == 0)
            return sym_StorageTypeNull;
        break;
    case 'O':
    switch (str[12]) {
    case 'b':
        if (strcmp(str + 13, "ject") == 0)
            return sym_StorageTypeObject;
        break;
    case 'p':
        if (strcmp(str + 13, "aquePointer") == 0)
            return sym_StorageTypeOpaquePointer;
        break;
    default: return -1;
    }
    case 'S':
    switch (str[12]) {
    case 't':
    switch (str[13]) {
    case 'a':
        if (strcmp(str + 14, "ck") == 0)
            return sym_StorageTypeStack;
        break;
    case 'r':
        if (strcmp(str + 14, "ing") == 0)
            return sym_StorageTypeString;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'T':
    switch (str[12]) {
    case 'e':
        if (strcmp(str + 13, "rm") == 0)
            return sym_StorageTypeTerm;
        break;
    case 'y':
        if (strcmp(str + 13, "pe") == 0)
            return sym_StorageTypeType;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'r':
        if (strcmp(str + 3, "uctType") == 0)
            return sym_StructType;
        break;
    default: return -1;
    }
    case 'u':
        if (strcmp(str + 2, "ccess") == 0)
            return sym_Success;
        break;
    default: return -1;
    }
    case 'T':
    switch (str[1]) {
    case 'e':
        if (strcmp(str + 2, "rmName") == 0)
            return sym_TermName;
        break;
    case 'o':
        if (strcmp(str + 2, "oManyInputs") == 0)
            return sym_TooManyInputs;
        break;
    case 'y':
        if (strcmp(str + 2, "peName") == 0)
            return sym_TypeName;
        break;
    default: return -1;
    }
    case 'U':
    switch (str[1]) {
    case 'n':
    switch (str[2]) {
    case 'c':
        if (strcmp(str + 3, "aptured") == 0)
            return sym_Uncaptured;
        break;
    case 'e':
        if (strcmp(str + 3, "valuated") == 0)
            return sym_Unevaluated;
        break;
    case 'i':
        if (strcmp(str + 3, "formListType") == 0)
            return sym_UniformListType;
        break;
    case 'k':
    switch (str[3]) {
    case 'n':
    switch (str[4]) {
    case 'o':
    switch (str[5]) {
    case 'w':
    switch (str[6]) {
    case 'n':
    switch (str[7]) {
    case 'I':
        if (strcmp(str + 8, "dentifier") == 0)
            return sym_UnknownIdentifier;
        break;
    case 0:
            return sym_Unknown;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 't':
        if (strcmp(str + 3, "yped") == 0)
            return sym_Untyped;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'W':
    switch (str[1]) {
    case 'h':
        if (strcmp(str + 2, "itespace") == 0)
            return sym_Whitespace;
        break;
    case 'i':
        if (strcmp(str + 2, "ldcard") == 0)
            return sym_Wildcard;
        break;
    default: return -1;
    }
    case 'Y':
        if (strcmp(str + 1, "es") == 0)
            return sym_Yes;
        break;
    case 's':
    switch (str[1]) {
    case 't':
    switch (str[2]) {
    case 'a':
    switch (str[3]) {
    case 't':
    switch (str[4]) {
    case '_':
    switch (str[5]) {
    case 'B':
        if (strcmp(str + 6, "lockNameLookups") == 0)
            return stat_BlockNameLookups;
        break;
    case 'C':
    switch (str[6]) {
    case 'a':
    switch (str[7]) {
    case 's':
    switch (str[8]) {
    case 't':
    switch (str[9]) {
    case '_':
    switch (str[10]) {
    case 'F':
        if (strcmp(str + 11, "inishFrame") == 0)
            return stat_Cast_FinishFrame;
        break;
    case 'L':
        if (strcmp(str + 11, "istCastElement") == 0)
            return stat_Cast_ListCastElement;
        break;
    case 'P':
        if (strcmp(str + 11, "ushFrameWithInputs") == 0)
            return stat_Cast_PushFrameWithInputs;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
    switch (str[7]) {
    case 'p':
    switch (str[8]) {
    case 'y':
    switch (str[9]) {
    case '_':
    switch (str[10]) {
    case 'L':
    switch (str[11]) {
    case 'i':
        if (strcmp(str + 12, "stDuplicate") == 0)
            return stat_Copy_ListDuplicate;
        break;
    case 'o':
        if (strcmp(str + 12, "opCopyRebound") == 0)
            return stat_Copy_LoopCopyRebound;
        break;
    default: return -1;
    }
    case 'P':
    switch (str[11]) {
    case 'u':
    switch (str[12]) {
    case 's':
    switch (str[13]) {
    case 'h':
    switch (str[14]) {
    case 'F':
        if (strcmp(str + 15, "rameWithInputs") == 0)
            return stat_Copy_PushFrameWithInputs;
        break;
    case 'e':
    switch (str[15]) {
    case 'd':
    switch (str[16]) {
    case 'I':
    switch (str[17]) {
    case 'n':
    switch (str[18]) {
    case 'p':
    switch (str[19]) {
    case 'u':
    switch (str[20]) {
    case 't':
    switch (str[21]) {
    case 'M':
        if (strcmp(str + 22, "ultiNewFrame") == 0)
            return stat_Copy_PushedInputMultiNewFrame;
        break;
    case 'N':
        if (strcmp(str + 22, "ewFrame") == 0)
            return stat_Copy_PushedInputNewFrame;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'D':
    switch (str[6]) {
    case 'i':
        if (strcmp(str + 7, "ctHardCopy") == 0)
            return stat_DictHardCopy;
        break;
    case 'y':
    switch (str[7]) {
    case 'n':
    switch (str[8]) {
    case 'a':
    switch (str[9]) {
    case 'm':
    switch (str[10]) {
    case 'i':
    switch (str[11]) {
    case 'c':
    switch (str[12]) {
    case 'C':
        if (strcmp(str + 13, "all") == 0)
            return stat_DynamicCall;
        break;
    case 'M':
        if (strcmp(str + 13, "ethodCall") == 0)
            return stat_DynamicMethodCall;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'F':
        if (strcmp(str + 6, "inishDynamicCall") == 0)
            return stat_FinishDynamicCall;
        break;
    case 'I':
    switch (str[6]) {
    case 'n':
    switch (str[7]) {
    case 't':
    switch (str[8]) {
    case 'e':
    switch (str[9]) {
    case 'r':
    switch (str[10]) {
    case 'n':
    switch (str[11]) {
    case 'e':
    switch (str[12]) {
    case 'd':
    switch (str[13]) {
    case 'N':
    switch (str[14]) {
    case 'a':
    switch (str[15]) {
    case 'm':
    switch (str[16]) {
    case 'e':
    switch (str[17]) {
    case 'C':
        if (strcmp(str + 18, "reate") == 0)
            return stat_InternedNameCreate;
        break;
    case 'L':
        if (strcmp(str + 18, "ookup") == 0)
            return stat_InternedNameLookup;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'p':
        if (strcmp(str + 11, "reterCastOutputFromFinishedFrame") == 0)
            return stat_InterpreterCastOutputFromFinishedFrame;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'L':
    switch (str[6]) {
    case 'i':
    switch (str[7]) {
    case 's':
    switch (str[8]) {
    case 't':
    switch (str[9]) {
    case 'H':
        if (strcmp(str + 10, "ardCopy") == 0)
            return stat_ListHardCopy;
        break;
    case 'S':
        if (strcmp(str + 10, "oftCopy") == 0)
            return stat_ListSoftCopy;
        break;
    case 's':
    switch (str[10]) {
    case 'C':
        if (strcmp(str + 11, "reated") == 0)
            return stat_ListsCreated;
        break;
    case 'G':
        if (strcmp(str + 11, "rown") == 0)
            return stat_ListsGrown;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
    switch (str[7]) {
    case 'o':
    switch (str[8]) {
    case 'p':
    switch (str[9]) {
    case 'F':
        if (strcmp(str + 10, "inishIteration") == 0)
            return stat_LoopFinishIteration;
        break;
    case 'W':
        if (strcmp(str + 10, "riteOutput") == 0)
            return stat_LoopWriteOutput;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'P':
        if (strcmp(str + 6, "ushFrame") == 0)
            return stat_PushFrame;
        break;
    case 'S':
    switch (str[6]) {
    case 'e':
    switch (str[7]) {
    case 't':
    switch (str[8]) {
    case 'F':
        if (strcmp(str + 9, "ield") == 0)
            return stat_SetField;
        break;
    case 'I':
        if (strcmp(str + 9, "ndex") == 0)
            return stat_SetIndex;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 't':
    switch (str[7]) {
    case 'e':
        if (strcmp(str + 8, "pInterpreter") == 0)
            return stat_StepInterpreter;
        break;
    case 'r':
    switch (str[8]) {
    case 'i':
    switch (str[9]) {
    case 'n':
    switch (str[10]) {
    case 'g':
    switch (str[11]) {
    case 'C':
        if (strcmp(str + 12, "reate") == 0)
            return stat_StringCreate;
        break;
    case 'D':
        if (strcmp(str + 12, "uplicate") == 0)
            return stat_StringDuplicate;
        break;
    case 'R':
    switch (str[12]) {
    case 'e':
    switch (str[13]) {
    case 's':
    switch (str[14]) {
    case 'i':
    switch (str[15]) {
    case 'z':
    switch (str[16]) {
    case 'e':
    switch (str[17]) {
    case 'C':
        if (strcmp(str + 18, "reate") == 0)
            return stat_StringResizeCreate;
        break;
    case 'I':
        if (strcmp(str + 18, "nPlace") == 0)
            return stat_StringResizeInPlace;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'S':
        if (strcmp(str + 12, "oftCopy") == 0)
            return stat_StringSoftCopy;
        break;
    case 'T':
        if (strcmp(str + 12, "oStd") == 0)
            return stat_StringToStd;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'T':
    switch (str[6]) {
    case 'e':
    switch (str[7]) {
    case 'r':
    switch (str[8]) {
    case 'm':
    switch (str[9]) {
    case 'P':
    switch (str[10]) {
    case 'r':
    switch (str[11]) {
    case 'o':
    switch (str[12]) {
    case 'p':
    switch (str[13]) {
    case 'A':
    switch (str[14]) {
    case 'c':
        if (strcmp(str + 15, "cess") == 0)
            return stat_TermPropAccess;
        break;
    case 'd':
        if (strcmp(str + 15, "ded") == 0)
            return stat_TermPropAdded;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 's':
        if (strcmp(str + 10, "Created") == 0)
            return stat_TermsCreated;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 7, "uch_ListCast") == 0)
            return stat_Touch_ListCast;
        break;
    default: return -1;
    }
    case 'V':
    switch (str[6]) {
    case 'a':
    switch (str[7]) {
    case 'l':
    switch (str[8]) {
    case 'u':
    switch (str[9]) {
    case 'e':
    switch (str[10]) {
    case 'C':
    switch (str[11]) {
    case 'a':
    switch (str[12]) {
    case 's':
    switch (str[13]) {
    case 't':
    switch (str[14]) {
    case 'D':
        if (strcmp(str + 15, "ispatched") == 0)
            return stat_ValueCastDispatched;
        break;
    case 0:
            return stat_ValueCast;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 12, "pies") == 0)
            return stat_ValueCopies;
        break;
    case 'r':
        if (strcmp(str + 12, "eates") == 0)
            return stat_ValueCreates;
        break;
    default: return -1;
    }
    case 'T':
        if (strcmp(str + 11, "ouch") == 0)
            return stat_ValueTouch;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'W':
        if (strcmp(str + 6, "riteTermBytecode") == 0)
            return stat_WriteTermBytecode;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 't':
    switch (str[1]) {
    case 'o':
    switch (str[2]) {
    case 'k':
    switch (str[3]) {
    case '_':
    switch (str[4]) {
    case 'A':
    switch (str[5]) {
    case 'm':
        if (strcmp(str + 6, "persand") == 0)
            return tok_Ampersand;
        break;
    case 'n':
        if (strcmp(str + 6, "d") == 0)
            return tok_And;
        break;
    case 't':
            return tok_At;
    default: return -1;
    }
    case 'B':
    switch (str[5]) {
    case 'o':
        if (strcmp(str + 6, "ol") == 0)
            return tok_Bool;
        break;
    case 'r':
        if (strcmp(str + 6, "eak") == 0)
            return tok_Break;
        break;
    default: return -1;
    }
    case 'C':
    switch (str[5]) {
    case 'a':
        if (strcmp(str + 6, "se") == 0)
            return tok_Case;
        break;
    case 'o':
    switch (str[6]) {
    case 'l':
    switch (str[7]) {
    case 'o':
    switch (str[8]) {
    case 'n':
    switch (str[9]) {
    case 'E':
        if (strcmp(str + 10, "quals") == 0)
            return tok_ColonEquals;
        break;
    case 'S':
        if (strcmp(str + 10, "tring") == 0)
            return tok_ColonString;
        break;
    case 0:
            return tok_Colon;
    default: return -1;
    }
    case 'r':
            return tok_Color;
    default: return -1;
    }
    default: return -1;
    }
    case 'm':
    switch (str[7]) {
    case 'm':
    switch (str[8]) {
    case 'a':
            return tok_Comma;
    case 'e':
        if (strcmp(str + 9, "nt") == 0)
            return tok_Comment;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'n':
        if (strcmp(str + 7, "tinue") == 0)
            return tok_Continue;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'D':
    switch (str[5]) {
    case 'e':
        if (strcmp(str + 6, "f") == 0)
            return tok_Def;
        break;
    case 'i':
        if (strcmp(str + 6, "scard") == 0)
            return tok_Discard;
        break;
    case 'o':
    switch (str[6]) {
    case 't':
    switch (str[7]) {
    case 'A':
        if (strcmp(str + 8, "t") == 0)
            return tok_DotAt;
        break;
    case 0:
            return tok_Dot;
    default: return -1;
    }
    case 'u':
    switch (str[7]) {
    case 'b':
    switch (str[8]) {
    case 'l':
    switch (str[9]) {
    case 'e':
    switch (str[10]) {
    case 'A':
        if (strcmp(str + 11, "mpersand") == 0)
            return tok_DoubleAmpersand;
        break;
    case 'C':
        if (strcmp(str + 11, "olon") == 0)
            return tok_DoubleColon;
        break;
    case 'E':
        if (strcmp(str + 11, "quals") == 0)
            return tok_DoubleEquals;
        break;
    case 'S':
    switch (str[11]) {
    case 'l':
        if (strcmp(str + 12, "ash") == 0)
            return tok_DoubleSlash;
        break;
    case 't':
        if (strcmp(str + 12, "ar") == 0)
            return tok_DoubleStar;
        break;
    default: return -1;
    }
    case 'V':
        if (strcmp(str + 11, "erticalBar") == 0)
            return tok_DoubleVerticalBar;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'E':
    switch (str[5]) {
    case 'l':
    switch (str[6]) {
    case 'i':
        if (strcmp(str + 7, "f") == 0)
            return tok_Elif;
        break;
    case 'l':
        if (strcmp(str + 7, "ipsis") == 0)
            return tok_Ellipsis;
        break;
    case 's':
        if (strcmp(str + 7, "e") == 0)
            return tok_Else;
        break;
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 6, "f") == 0)
            return tok_Eof;
        break;
    case 'q':
        if (strcmp(str + 6, "uals") == 0)
            return tok_Equals;
        break;
    default: return -1;
    }
    case 'F':
    switch (str[5]) {
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
    default: return -1;
    }
    case 'G':
    switch (str[5]) {
    case 'T':
    switch (str[6]) {
    case 'h':
    switch (str[7]) {
    case 'a':
    switch (str[8]) {
    case 'n':
    switch (str[9]) {
    case 'E':
        if (strcmp(str + 10, "q") == 0)
            return tok_GThanEq;
        break;
    case 0:
            return tok_GThan;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'H':
        if (strcmp(str + 5, "exInteger") == 0)
            return tok_HexInteger;
        break;
    case 'I':
    switch (str[5]) {
    case 'd':
        if (strcmp(str + 6, "entifier") == 0)
            return tok_Identifier;
        break;
    case 'f':
            return tok_If;
    case 'n':
    switch (str[6]) {
    case 'c':
        if (strcmp(str + 7, "lude") == 0)
            return tok_Include;
        break;
    case 't':
        if (strcmp(str + 7, "eger") == 0)
            return tok_Integer;
        break;
    case 0:
            return tok_In;
    default: return -1;
    }
    default: return -1;
    }
    case 'L':
    switch (str[5]) {
    case 'B':
    switch (str[6]) {
    case 'r':
    switch (str[7]) {
    case 'a':
    switch (str[8]) {
    case 'c':
    switch (str[9]) {
    case 'e':
            return tok_LBrace;
    case 'k':
        if (strcmp(str + 10, "et") == 0)
            return tok_LBracket;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'P':
        if (strcmp(str + 6, "aren") == 0)
            return tok_LParen;
        break;
    case 'T':
    switch (str[6]) {
    case 'h':
    switch (str[7]) {
    case 'a':
    switch (str[8]) {
    case 'n':
    switch (str[9]) {
    case 'E':
        if (strcmp(str + 10, "q") == 0)
            return tok_LThanEq;
        break;
    case 0:
            return tok_LThan;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'e':
        if (strcmp(str + 6, "ftArrow") == 0)
            return tok_LeftArrow;
        break;
    default: return -1;
    }
    case 'M':
    switch (str[5]) {
    case 'i':
    switch (str[6]) {
    case 'n':
    switch (str[7]) {
    case 'u':
    switch (str[8]) {
    case 's':
    switch (str[9]) {
    case 'E':
        if (strcmp(str + 10, "quals") == 0)
            return tok_MinusEquals;
        break;
    case 0:
            return tok_Minus;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'N':
    switch (str[5]) {
    case 'a':
        if (strcmp(str + 6, "mespace") == 0)
            return tok_Namespace;
        break;
    case 'e':
        if (strcmp(str + 6, "wline") == 0)
            return tok_Newline;
        break;
    case 'o':
    switch (str[6]) {
    case 't':
    switch (str[7]) {
    case 'E':
        if (strcmp(str + 8, "quals") == 0)
            return tok_NotEquals;
        break;
    case 0:
            return tok_Not;
    default: return -1;
    }
    default: return -1;
    }
    case 'u':
        if (strcmp(str + 6, "ll") == 0)
            return tok_Null;
        break;
    default: return -1;
    }
    case 'O':
        if (strcmp(str + 5, "r") == 0)
            return tok_Or;
        break;
    case 'P':
    switch (str[5]) {
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
    case 'u':
    switch (str[7]) {
    case 's':
    switch (str[8]) {
    case 'E':
        if (strcmp(str + 9, "quals") == 0)
            return tok_PlusEquals;
        break;
    case 0:
            return tok_Plus;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 6, "und") == 0)
            return tok_Pound;
        break;
    default: return -1;
    }
    case 'Q':
        if (strcmp(str + 5, "uestion") == 0)
            return tok_Question;
        break;
    case 'R':
    switch (str[5]) {
    case 'B':
    switch (str[6]) {
    case 'r':
    switch (str[7]) {
    case 'a':
    switch (str[8]) {
    case 'c':
    switch (str[9]) {
    case 'e':
            return tok_RBrace;
    case 'k':
        if (strcmp(str + 10, "et") == 0)
            return tok_RBracket;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'P':
        if (strcmp(str + 6, "aren") == 0)
            return tok_RParen;
        break;
    case 'e':
    switch (str[6]) {
    case 'q':
        if (strcmp(str + 7, "uire") == 0)
            return tok_Require;
        break;
    case 't':
        if (strcmp(str + 7, "urn") == 0)
            return tok_Return;
        break;
    default: return -1;
    }
    case 'i':
        if (strcmp(str + 6, "ghtArrow") == 0)
            return tok_RightArrow;
        break;
    default: return -1;
    }
    case 'S':
    switch (str[5]) {
    case 'e':
    switch (str[6]) {
    case 'c':
        if (strcmp(str + 7, "tion") == 0)
            return tok_Section;
        break;
    case 'm':
        if (strcmp(str + 7, "icolon") == 0)
            return tok_Semicolon;
        break;
    default: return -1;
    }
    case 'l':
    switch (str[6]) {
    case 'a':
    switch (str[7]) {
    case 's':
    switch (str[8]) {
    case 'h':
    switch (str[9]) {
    case 'E':
        if (strcmp(str + 10, "quals") == 0)
            return tok_SlashEquals;
        break;
    case 0:
            return tok_Slash;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 't':
    switch (str[6]) {
    case 'a':
    switch (str[7]) {
    case 'r':
    switch (str[8]) {
    case 'E':
        if (strcmp(str + 9, "quals") == 0)
            return tok_StarEquals;
        break;
    case 0:
            return tok_Star;
    default: return -1;
    }
    case 't':
        if (strcmp(str + 8, "e") == 0)
            return tok_State;
        break;
    default: return -1;
    }
    case 'r':
        if (strcmp(str + 7, "ing") == 0)
            return tok_String;
        break;
    default: return -1;
    }
    case 'w':
        if (strcmp(str + 6, "itch") == 0)
            return tok_Switch;
        break;
    default: return -1;
    }
    case 'T':
    switch (str[5]) {
    case 'r':
    switch (str[6]) {
    case 'i':
    switch (str[7]) {
    case 'p':
    switch (str[8]) {
    case 'l':
    switch (str[9]) {
    case 'e':
    switch (str[10]) {
    case 'G':
        if (strcmp(str + 11, "Than") == 0)
            return tok_TripleGThan;
        break;
    case 'L':
        if (strcmp(str + 11, "Than") == 0)
            return tok_TripleLThan;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'u':
        if (strcmp(str + 7, "e") == 0)
            return tok_True;
        break;
    default: return -1;
    }
    case 'w':
        if (strcmp(str + 6, "oDots") == 0)
            return tok_TwoDots;
        break;
    case 'y':
        if (strcmp(str + 6, "pe") == 0)
            return tok_Type;
        break;
    default: return -1;
    }
    case 'U':
    switch (str[5]) {
    case 'n':
    switch (str[6]) {
    case 'r':
        if (strcmp(str + 7, "ecognized") == 0)
            return tok_Unrecognized;
        break;
    case 'u':
    switch (str[7]) {
    case 's':
    switch (str[8]) {
    case 'e':
    switch (str[9]) {
    case 'd':
    switch (str[10]) {
    case 'N':
    switch (str[11]) {
    case 'a':
    switch (str[12]) {
    case 'm':
    switch (str[13]) {
    case 'e':
    switch (str[14]) {
    case '1':
            return tok_UnusedName1;
    case '2':
            return tok_UnusedName2;
    case '3':
            return tok_UnusedName3;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'W':
    switch (str[5]) {
    case 'h':
    switch (str[6]) {
    case 'i':
    switch (str[7]) {
    case 'l':
        if (strcmp(str + 8, "e") == 0)
            return tok_While;
        break;
    case 't':
        if (strcmp(str + 8, "espace") == 0)
            return tok_Whitespace;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }

    return -1;
}
} // namespace circa
