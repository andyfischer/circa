// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#include "common_headers.h"
#include "names_builtin.h"

namespace circa {

const char* builtin_symbol_to_string(int name)
{
    if (name >= s_LastBuiltinName)
        return NULL;

    switch (name) {
    case s_addr: return "addr";
    case s_advance: return "advance";
    case s_block: return "block";
    case s_break: return "break";
    case s_case: return "case";
    case s_continue: return "continue";
    case s_conditional_done: return "conditional_done";
    case s_count: return "count";
    case s_current_term: return "current_term";
    case s_declared_state: return "declared_state";
    case s_discard: return "discard";
    case s_done: return "done";
    case s_error: return "error";
    case s_expr: return "expr";
    case s_failure: return "failure";
    case s_file: return "file";
    case s_filename: return "filename";
    case s_format: return "format";
    case s_frames: return "frames";
    case s_has_state: return "has_state";
    case s_ident: return "ident";
    case s_incoming: return "incoming";
    case s_index: return "index";
    case s_inputs: return "inputs";
    case s_invalid: return "invalid";
    case s_iterator: return "iterator";
    case s_iterator_value: return "iterator_value";
    case s_items: return "items";
    case s_key: return "key";
    case s_last: return "last";
    case s_list: return "list";
    case s_liveness_list: return "liveness_list";
    case s_local_state: return "local_state";
    case s_local_state_key: return "local_state_key";
    case s_loop: return "loop";
    case s_maddr: return "maddr";
    case s_major_block: return "major_block";
    case s_maybe: return "maybe";
    case s_message: return "message";
    case s_methodCache: return "methodCache";
    case s_next: return "next";
    case s_next_case: return "next_case";
    case s_no: return "no";
    case s_none: return "none";
    case s_newline: return "newline";
    case s_out: return "out";
    case s_outputSlot: return "outputSlot";
    case s_outgoing: return "outgoing";
    case s_pc: return "pc";
    case s_produceOutput: return "produceOutput";
    case s_repeat: return "repeat";
    case s_return: return "return";
    case s_slot: return "slot";
    case s_slots: return "slots";
    case s_slotCount: return "slotCount";
    case s_switch: return "switch";
    case s_success: return "success";
    case s_type: return "type";
    case s_term: return "term";
    case s_top: return "top";
    case s_unknown: return "unknown";
    case s_yes: return "yes";
    case s_EvaluationEmpty: return "EvaluationEmpty";
    case s_HasEffects: return "HasEffects";
    case s_HasControlFlow: return "HasControlFlow";
    case s_HasDynamicDispatch: return "HasDynamicDispatch";
    case s_DirtyStateType: return "DirtyStateType";
    case s_Builtins: return "Builtins";
    case s_ModuleName: return "ModuleName";
    case s_StaticErrors: return "StaticErrors";
    case s_IsModule: return "IsModule";
    case s_AccumulatingOutput: return "AccumulatingOutput";
    case s_Comment: return "Comment";
    case s_Constructor: return "Constructor";
    case s_Error: return "Error";
    case s_ExplicitState: return "ExplicitState";
    case s_ExplicitType: return "ExplicitType";
    case s_Field: return "Field";
    case s_FieldAccessor: return "FieldAccessor";
    case s_Final: return "Final";
    case s_Hidden: return "Hidden";
    case s_HiddenInput: return "HiddenInput";
    case s_Implicit: return "Implicit";
    case s_IgnoreError: return "IgnoreError";
    case s_LocalStateResult: return "LocalStateResult";
    case s_Meta: return "Meta";
    case s_Message: return "Message";
    case s_MethodName: return "MethodName";
    case s_ModifyList: return "ModifyList";
    case s_Multiple: return "Multiple";
    case s_Mutability: return "Mutability";
    case s_Optional: return "Optional";
    case s_OriginalText: return "OriginalText";
    case s_OverloadedFunc: return "OverloadedFunc";
    case s_Ref: return "Ref";
    case s_Rebind: return "Rebind";
    case s_RebindsInput: return "RebindsInput";
    case s_Setter: return "Setter";
    case s_State: return "State";
    case s_Step: return "Step";
    case s_Statement: return "Statement";
    case s_Output: return "Output";
    case s_PreferSpecialize: return "PreferSpecialize";
    case s_Error_UnknownType: return "Error_UnknownType";
    case s_Syntax_AnonFunction: return "Syntax_AnonFunction";
    case s_Syntax_BlockStyle: return "Syntax_BlockStyle";
    case s_Syntax_Brackets: return "Syntax_Brackets";
    case s_Syntax_ColorFormat: return "Syntax_ColorFormat";
    case s_Syntax_DeclarationStyle: return "Syntax_DeclarationStyle";
    case s_Syntax_ExplicitType: return "Syntax_ExplicitType";
    case s_Syntax_FunctionName: return "Syntax_FunctionName";
    case s_Syntax_IdentifierRebind: return "Syntax_IdentifierRebind";
    case s_Syntax_ImplicitName: return "Syntax_ImplicitName";
    case s_Syntax_Import: return "Syntax_Import";
    case s_Syntax_InputFormat: return "Syntax_InputFormat";
    case s_Syntax_IntegerFormat: return "Syntax_IntegerFormat";
    case s_Syntax_LineEnding: return "Syntax_LineEnding";
    case s_Syntax_LiteralList: return "Syntax_LiteralList";
    case s_Syntax_MethodDecl: return "Syntax_MethodDecl";
    case s_Syntax_Multiline: return "Syntax_Multiline";
    case s_Syntax_NameBinding: return "Syntax_NameBinding";
    case s_Syntax_NoBrackets: return "Syntax_NoBrackets";
    case s_Syntax_NoParens: return "Syntax_NoParens";
    case s_Syntax_Operator: return "Syntax_Operator";
    case s_Syntax_OriginalFormat: return "Syntax_OriginalFormat";
    case s_Syntax_Parens: return "Syntax_Parens";
    case s_Syntax_PreWs: return "Syntax_PreWs";
    case s_Syntax_PreDotWs: return "Syntax_PreDotWs";
    case s_Syntax_PreOperatorWs: return "Syntax_PreOperatorWs";
    case s_Syntax_PreEndWs: return "Syntax_PreEndWs";
    case s_Syntax_PreEqualsSpace: return "Syntax_PreEqualsSpace";
    case s_Syntax_PreLBracketWs: return "Syntax_PreLBracketWs";
    case s_Syntax_PreRBracketWs: return "Syntax_PreRBracketWs";
    case s_Syntax_PostEqualsSpace: return "Syntax_PostEqualsSpace";
    case s_Syntax_PostFunctionWs: return "Syntax_PostFunctionWs";
    case s_Syntax_PostKeywordWs: return "Syntax_PostKeywordWs";
    case s_Syntax_PostLBracketWs: return "Syntax_PostLBracketWs";
    case s_Syntax_PostHeadingWs: return "Syntax_PostHeadingWs";
    case s_Syntax_PostNameWs: return "Syntax_PostNameWs";
    case s_Syntax_PostWs: return "Syntax_PostWs";
    case s_Syntax_PostOperatorWs: return "Syntax_PostOperatorWs";
    case s_Syntax_Properties: return "Syntax_Properties";
    case s_Syntax_QuoteType: return "Syntax_QuoteType";
    case s_Syntax_RebindSymbol: return "Syntax_RebindSymbol";
    case s_Syntax_RebindOperator: return "Syntax_RebindOperator";
    case s_Syntax_RebindingInfix: return "Syntax_RebindingInfix";
    case s_Syntax_ReturnStatement: return "Syntax_ReturnStatement";
    case s_Syntax_Require: return "Syntax_Require";
    case s_Syntax_StateKeyword: return "Syntax_StateKeyword";
    case s_Syntax_TypeMagicSymbol: return "Syntax_TypeMagicSymbol";
    case s_Syntax_WhitespaceBeforeEnd: return "Syntax_WhitespaceBeforeEnd";
    case s_Syntax_WhitespacePreColon: return "Syntax_WhitespacePreColon";
    case s_Syntax_WhitespacePostColon: return "Syntax_WhitespacePostColon";
    case s_Wildcard: return "Wildcard";
    case s_RecursiveWildcard: return "RecursiveWildcard";
    case s_Function: return "Function";
    case s_TypeRelease: return "TypeRelease";
    case s_FileNotFound: return "FileNotFound";
    case s_NotEnoughInputs: return "NotEnoughInputs";
    case s_TooManyInputs: return "TooManyInputs";
    case s_ExtraOutputNotFound: return "ExtraOutputNotFound";
    case s_Default: return "Default";
    case s_ByDemand: return "ByDemand";
    case s_Unevaluated: return "Unevaluated";
    case s_InProgress: return "InProgress";
    case s_Lazy: return "Lazy";
    case s_Consumed: return "Consumed";
    case s_Uncaptured: return "Uncaptured";
    case s_Return: return "Return";
    case s_Continue: return "Continue";
    case s_Break: return "Break";
    case s_Discard: return "Discard";
    case s_Control: return "Control";
    case s_ExitLevelFunction: return "ExitLevelFunction";
    case s_ExitLevelLoop: return "ExitLevelLoop";
    case s_HighestExitLevel: return "HighestExitLevel";
    case s_ExtraReturn: return "ExtraReturn";
    case s_Name: return "Name";
    case s_Primary: return "Primary";
    case s_Anonymous: return "Anonymous";
    case s_Entropy: return "Entropy";
    case s_OnDemand: return "OnDemand";
    case s_hacks: return "hacks";
    case s_no_effect: return "no_effect";
    case s_no_save_state: return "no_save_state";
    case s_effect: return "effect";
    case s_set_value: return "set_value";
    case s_watch: return "watch";
    case s_Copy: return "Copy";
    case s_Move: return "Move";
    case s_Unobservable: return "Unobservable";
    case s_TermCounter: return "TermCounter";
    case s_Watch: return "Watch";
    case s_StackReady: return "StackReady";
    case s_StackRunning: return "StackRunning";
    case s_StackFinished: return "StackFinished";
    case s_InfixOperator: return "InfixOperator";
    case s_FunctionName: return "FunctionName";
    case s_TypeName: return "TypeName";
    case s_TermName: return "TermName";
    case s_Keyword: return "Keyword";
    case s_Whitespace: return "Whitespace";
    case s_UnknownIdentifier: return "UnknownIdentifier";
    case s_LookupAny: return "LookupAny";
    case s_LookupType: return "LookupType";
    case s_LookupFunction: return "LookupFunction";
    case s_Untyped: return "Untyped";
    case s_UniformListType: return "UniformListType";
    case s_AnonStructType: return "AnonStructType";
    case s_StructType: return "StructType";
    case s_NativePatch: return "NativePatch";
    case s_RecompileModule: return "RecompileModule";
    case s_Filesystem: return "Filesystem";
    case s_Tarball: return "Tarball";
    case s_Bootstrapping: return "Bootstrapping";
    case s_Done: return "Done";
    case s_StorageTypeNull: return "StorageTypeNull";
    case s_StorageTypeInt: return "StorageTypeInt";
    case s_StorageTypeFloat: return "StorageTypeFloat";
    case s_StorageTypeBlob: return "StorageTypeBlob";
    case s_StorageTypeBool: return "StorageTypeBool";
    case s_StorageTypeStack: return "StorageTypeStack";
    case s_StorageTypeString: return "StorageTypeString";
    case s_StorageTypeList: return "StorageTypeList";
    case s_StorageTypeOpaquePointer: return "StorageTypeOpaquePointer";
    case s_StorageTypeTerm: return "StorageTypeTerm";
    case s_StorageTypeType: return "StorageTypeType";
    case s_StorageTypeHandle: return "StorageTypeHandle";
    case s_StorageTypeHashtable: return "StorageTypeHashtable";
    case s_StorageTypeObject: return "StorageTypeObject";
    case s_InterfaceType: return "InterfaceType";
    case s_Delete: return "Delete";
    case s_Insert: return "Insert";
    case s_Element: return "Element";
    case s_Key: return "Key";
    case s_Replace: return "Replace";
    case s_Append: return "Append";
    case s_Truncate: return "Truncate";
    case s_ChangeAppend: return "ChangeAppend";
    case s_ChangeRename: return "ChangeRename";
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
    case tok_LSquare: return "tok_LSquare";
    case tok_RSquare: return "tok_RSquare";
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
    case tok_FatArrow: return "tok_FatArrow";
    case tok_LeftArrow: return "tok_LeftArrow";
    case tok_Ampersand: return "tok_Ampersand";
    case tok_DoubleAmpersand: return "tok_DoubleAmpersand";
    case tok_VerticalBar: return "tok_VerticalBar";
    case tok_DoubleVerticalBar: return "tok_DoubleVerticalBar";
    case tok_Semicolon: return "tok_Semicolon";
    case tok_TwoDots: return "tok_TwoDots";
    case tok_Ellipsis: return "tok_Ellipsis";
    case tok_TripleLThan: return "tok_TripleLThan";
    case tok_TripleGThan: return "tok_TripleGThan";
    case tok_Pound: return "tok_Pound";
    case tok_Def: return "tok_Def";
    case tok_Struct: return "tok_Struct";
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
    case tok_Let: return "tok_Let";
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
    case tok_Import: return "tok_Import";
    case tok_Package: return "tok_Package";
    case tok_Section: return "tok_Section";
    case tok_Whitespace: return "tok_Whitespace";
    case tok_Newline: return "tok_Newline";
    case tok_Comment: return "tok_Comment";
    case tok_Eof: return "tok_Eof";
    case tok_Unrecognized: return "tok_Unrecognized";
    case s_NormalCall: return "NormalCall";
    case s_FuncApply: return "FuncApply";
    case s_FuncCall: return "FuncCall";
    case s_FirstStatIndex: return "FirstStatIndex";
    case stat_TermCreated: return "stat_TermCreated";
    case stat_TermPropAdded: return "stat_TermPropAdded";
    case stat_TermPropAccess: return "stat_TermPropAccess";
    case stat_NameSearch: return "stat_NameSearch";
    case stat_NameSearchStep: return "stat_NameSearchStep";
    case stat_FindModule: return "stat_FindModule";
    case stat_Bytecode_WriteTerm: return "stat_Bytecode_WriteTerm";
    case stat_Bytecode_CreateEntry: return "stat_Bytecode_CreateEntry";
    case stat_LoadFrameState: return "stat_LoadFrameState";
    case stat_StoreFrameState: return "stat_StoreFrameState";
    case stat_AppendMove: return "stat_AppendMove";
    case stat_GetIndexCopy: return "stat_GetIndexCopy";
    case stat_GetIndexMove: return "stat_GetIndexMove";
    case stat_Interpreter_Step: return "stat_Interpreter_Step";
    case stat_Interpreter_DynamicMethod_CacheHit: return "stat_Interpreter_DynamicMethod_CacheHit";
    case stat_Interpreter_DynamicMethod_SlowLookup: return "stat_Interpreter_DynamicMethod_SlowLookup";
    case stat_Interpreter_DynamicMethod_SlowLookup_Module: return "stat_Interpreter_DynamicMethod_SlowLookup_Module";
    case stat_Interpreter_DynamicMethod_SlowLookup_Hashtable: return "stat_Interpreter_DynamicMethod_SlowLookup_Hashtable";
    case stat_Interpreter_DynamicMethod_ModuleLookup: return "stat_Interpreter_DynamicMethod_ModuleLookup";
    case stat_Interpreter_DynamicFuncToClosureCall: return "stat_Interpreter_DynamicFuncToClosureCall";
    case stat_Interpreter_CopyTermValue: return "stat_Interpreter_CopyTermValue";
    case stat_Interpreter_CopyStackValue: return "stat_Interpreter_CopyStackValue";
    case stat_Interpreter_MoveStackValue: return "stat_Interpreter_MoveStackValue";
    case stat_Interpreter_CopyConst: return "stat_Interpreter_CopyConst";
    case stat_FindEnvValue: return "stat_FindEnvValue";
    case stat_Make: return "stat_Make";
    case stat_Copy: return "stat_Copy";
    case stat_Cast: return "stat_Cast";
    case stat_ValueCastDispatched: return "stat_ValueCastDispatched";
    case stat_Touch: return "stat_Touch";
    case stat_ListsCreated: return "stat_ListsCreated";
    case stat_ListsGrown: return "stat_ListsGrown";
    case stat_ListSoftCopy: return "stat_ListSoftCopy";
    case stat_ListDuplicate: return "stat_ListDuplicate";
    case stat_ListDuplicate_100Count: return "stat_ListDuplicate_100Count";
    case stat_ListDuplicate_ElementCopy: return "stat_ListDuplicate_ElementCopy";
    case stat_ListCast_Touch: return "stat_ListCast_Touch";
    case stat_ListCast_CastElement: return "stat_ListCast_CastElement";
    case stat_HashtableDuplicate: return "stat_HashtableDuplicate";
    case stat_HashtableDuplicate_Copy: return "stat_HashtableDuplicate_Copy";
    case stat_StringCreate: return "stat_StringCreate";
    case stat_StringDuplicate: return "stat_StringDuplicate";
    case stat_StringResizeInPlace: return "stat_StringResizeInPlace";
    case stat_StringResizeCreate: return "stat_StringResizeCreate";
    case stat_StringSoftCopy: return "stat_StringSoftCopy";
    case stat_StringToStd: return "stat_StringToStd";
    case stat_DynamicCall: return "stat_DynamicCall";
    case stat_FinishDynamicCall: return "stat_FinishDynamicCall";
    case stat_DynamicMethodCall: return "stat_DynamicMethodCall";
    case stat_SetIndex: return "stat_SetIndex";
    case stat_SetField: return "stat_SetField";
    case stat_SetWithSelector_Touch_List: return "stat_SetWithSelector_Touch_List";
    case stat_SetWithSelector_Touch_Hashtable: return "stat_SetWithSelector_Touch_Hashtable";
    case stat_StackPushFrame: return "stat_StackPushFrame";
    case s_LastStatIndex: return "LastStatIndex";
    case s_LastBuiltinName: return "LastBuiltinName";
    default: return NULL;
    }
}

int builtin_symbol_from_string(const char* str)
{
    switch (str[0]) {
    case 'A':
    switch (str[1]) {
    case 'c':
        if (strcmp(str + 2, "cumulatingOutput") == 0)
            return s_AccumulatingOutput;
        break;
    case 'n':
    switch (str[2]) {
    case 'o':
    switch (str[3]) {
    case 'n':
    switch (str[4]) {
    case 'S':
        if (strcmp(str + 5, "tructType") == 0)
            return s_AnonStructType;
        break;
    case 'y':
        if (strcmp(str + 5, "mous") == 0)
            return s_Anonymous;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'p':
        if (strcmp(str + 2, "pend") == 0)
            return s_Append;
        break;
    default: return -1;
    }
    case 'B':
    switch (str[1]) {
    case 'o':
        if (strcmp(str + 2, "otstrapping") == 0)
            return s_Bootstrapping;
        break;
    case 'r':
        if (strcmp(str + 2, "eak") == 0)
            return s_Break;
        break;
    case 'u':
        if (strcmp(str + 2, "iltins") == 0)
            return s_Builtins;
        break;
    case 'y':
        if (strcmp(str + 2, "Demand") == 0)
            return s_ByDemand;
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
            return s_ChangeAppend;
        break;
    case 'R':
        if (strcmp(str + 7, "ename") == 0)
            return s_ChangeRename;
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
    case 'm':
        if (strcmp(str + 3, "ment") == 0)
            return s_Comment;
        break;
    case 'n':
    switch (str[3]) {
    case 's':
    switch (str[4]) {
    case 't':
        if (strcmp(str + 5, "ructor") == 0)
            return s_Constructor;
        break;
    case 'u':
        if (strcmp(str + 5, "med") == 0)
            return s_Consumed;
        break;
    default: return -1;
    }
    case 't':
    switch (str[4]) {
    case 'i':
        if (strcmp(str + 5, "nue") == 0)
            return s_Continue;
        break;
    case 'r':
        if (strcmp(str + 5, "ol") == 0)
            return s_Control;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'p':
        if (strcmp(str + 3, "y") == 0)
            return s_Copy;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'D':
    switch (str[1]) {
    case 'e':
    switch (str[2]) {
    case 'f':
        if (strcmp(str + 3, "ault") == 0)
            return s_Default;
        break;
    case 'l':
        if (strcmp(str + 3, "ete") == 0)
            return s_Delete;
        break;
    default: return -1;
    }
    case 'i':
    switch (str[2]) {
    case 'r':
        if (strcmp(str + 3, "tyStateType") == 0)
            return s_DirtyStateType;
        break;
    case 's':
        if (strcmp(str + 3, "card") == 0)
            return s_Discard;
        break;
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 2, "ne") == 0)
            return s_Done;
        break;
    default: return -1;
    }
    case 'E':
    switch (str[1]) {
    case 'l':
        if (strcmp(str + 2, "ement") == 0)
            return s_Element;
        break;
    case 'n':
        if (strcmp(str + 2, "tropy") == 0)
            return s_Entropy;
        break;
    case 'r':
    switch (str[2]) {
    case 'r':
    switch (str[3]) {
    case 'o':
    switch (str[4]) {
    case 'r':
    switch (str[5]) {
    case '_':
        if (strcmp(str + 6, "UnknownType") == 0)
            return s_Error_UnknownType;
        break;
    case 0:
            return s_Error;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'v':
        if (strcmp(str + 2, "aluationEmpty") == 0)
            return s_EvaluationEmpty;
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
            return s_ExitLevelFunction;
        break;
    case 'L':
        if (strcmp(str + 10, "oop") == 0)
            return s_ExitLevelLoop;
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
    switch (str[3]) {
    case 'l':
    switch (str[4]) {
    case 'i':
    switch (str[5]) {
    case 'c':
    switch (str[6]) {
    case 'i':
    switch (str[7]) {
    case 't':
    switch (str[8]) {
    case 'S':
        if (strcmp(str + 9, "tate") == 0)
            return s_ExplicitState;
        break;
    case 'T':
        if (strcmp(str + 9, "ype") == 0)
            return s_ExplicitType;
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
    case 't':
    switch (str[3]) {
    case 'r':
    switch (str[4]) {
    case 'a':
    switch (str[5]) {
    case 'O':
        if (strcmp(str + 6, "utputNotFound") == 0)
            return s_ExtraOutputNotFound;
        break;
    case 'R':
        if (strcmp(str + 6, "eturn") == 0)
            return s_ExtraReturn;
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
    case 'i':
    switch (str[2]) {
    case 'e':
    switch (str[3]) {
    case 'l':
    switch (str[4]) {
    case 'd':
    switch (str[5]) {
    case 'A':
        if (strcmp(str + 6, "ccessor") == 0)
            return s_FieldAccessor;
        break;
    case 0:
            return s_Field;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'l':
    switch (str[3]) {
    case 'e':
    switch (str[4]) {
    case 'N':
        if (strcmp(str + 5, "otFound") == 0)
            return s_FileNotFound;
        break;
    case 's':
        if (strcmp(str + 5, "ystem") == 0)
            return s_Filesystem;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'n':
        if (strcmp(str + 3, "al") == 0)
            return s_Final;
        break;
    case 'r':
        if (strcmp(str + 3, "stStatIndex") == 0)
            return s_FirstStatIndex;
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
            return s_FuncApply;
        break;
    case 'C':
        if (strcmp(str + 5, "all") == 0)
            return s_FuncCall;
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
            return s_FunctionName;
        break;
    case 0:
            return s_Function;
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
            return s_HasControlFlow;
        break;
    case 'D':
        if (strcmp(str + 4, "ynamicDispatch") == 0)
            return s_HasDynamicDispatch;
        break;
    case 'E':
        if (strcmp(str + 4, "ffects") == 0)
            return s_HasEffects;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'i':
    switch (str[2]) {
    case 'd':
    switch (str[3]) {
    case 'd':
    switch (str[4]) {
    case 'e':
    switch (str[5]) {
    case 'n':
    switch (str[6]) {
    case 'I':
        if (strcmp(str + 7, "nput") == 0)
            return s_HiddenInput;
        break;
    case 0:
            return s_Hidden;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'g':
        if (strcmp(str + 3, "hestExitLevel") == 0)
            return s_HighestExitLevel;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'I':
    switch (str[1]) {
    case 'g':
        if (strcmp(str + 2, "noreError") == 0)
            return s_IgnoreError;
        break;
    case 'm':
        if (strcmp(str + 2, "plicit") == 0)
            return s_Implicit;
        break;
    case 'n':
    switch (str[2]) {
    case 'P':
        if (strcmp(str + 3, "rogress") == 0)
            return s_InProgress;
        break;
    case 'f':
        if (strcmp(str + 3, "ixOperator") == 0)
            return s_InfixOperator;
        break;
    case 's':
        if (strcmp(str + 3, "ert") == 0)
            return s_Insert;
        break;
    case 't':
        if (strcmp(str + 3, "erfaceType") == 0)
            return s_InterfaceType;
        break;
    default: return -1;
    }
    case 's':
        if (strcmp(str + 2, "Module") == 0)
            return s_IsModule;
        break;
    default: return -1;
    }
    case 'K':
    switch (str[1]) {
    case 'e':
    switch (str[2]) {
    case 'y':
    switch (str[3]) {
    case 'w':
        if (strcmp(str + 4, "ord") == 0)
            return s_Keyword;
        break;
    case 0:
            return s_Key;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
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
            return s_LastBuiltinName;
        break;
    case 'S':
        if (strcmp(str + 5, "tatIndex") == 0)
            return s_LastStatIndex;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'z':
        if (strcmp(str + 3, "y") == 0)
            return s_Lazy;
        break;
    default: return -1;
    }
    case 'o':
    switch (str[2]) {
    case 'c':
        if (strcmp(str + 3, "alStateResult") == 0)
            return s_LocalStateResult;
        break;
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
            return s_LookupAny;
        break;
    case 'F':
        if (strcmp(str + 7, "unction") == 0)
            return s_LookupFunction;
        break;
    case 'T':
        if (strcmp(str + 7, "ype") == 0)
            return s_LookupType;
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
    switch (str[1]) {
    case 'e':
    switch (str[2]) {
    case 's':
        if (strcmp(str + 3, "sage") == 0)
            return s_Message;
        break;
    case 't':
    switch (str[3]) {
    case 'a':
            return s_Meta;
    case 'h':
        if (strcmp(str + 4, "odName") == 0)
            return s_MethodName;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
    switch (str[2]) {
    case 'd':
    switch (str[3]) {
    case 'i':
        if (strcmp(str + 4, "fyList") == 0)
            return s_ModifyList;
        break;
    case 'u':
        if (strcmp(str + 4, "leName") == 0)
            return s_ModuleName;
        break;
    default: return -1;
    }
    case 'v':
        if (strcmp(str + 3, "e") == 0)
            return s_Move;
        break;
    default: return -1;
    }
    case 'u':
    switch (str[2]) {
    case 'l':
        if (strcmp(str + 3, "tiple") == 0)
            return s_Multiple;
        break;
    case 't':
        if (strcmp(str + 3, "ability") == 0)
            return s_Mutability;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'N':
    switch (str[1]) {
    case 'a':
    switch (str[2]) {
    case 'm':
        if (strcmp(str + 3, "e") == 0)
            return s_Name;
        break;
    case 't':
        if (strcmp(str + 3, "ivePatch") == 0)
            return s_NativePatch;
        break;
    default: return -1;
    }
    case 'o':
    switch (str[2]) {
    case 'r':
        if (strcmp(str + 3, "malCall") == 0)
            return s_NormalCall;
        break;
    case 't':
        if (strcmp(str + 3, "EnoughInputs") == 0)
            return s_NotEnoughInputs;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'O':
    switch (str[1]) {
    case 'n':
        if (strcmp(str + 2, "Demand") == 0)
            return s_OnDemand;
        break;
    case 'p':
        if (strcmp(str + 2, "tional") == 0)
            return s_Optional;
        break;
    case 'r':
        if (strcmp(str + 2, "iginalText") == 0)
            return s_OriginalText;
        break;
    case 'u':
        if (strcmp(str + 2, "tput") == 0)
            return s_Output;
        break;
    case 'v':
        if (strcmp(str + 2, "erloadedFunc") == 0)
            return s_OverloadedFunc;
        break;
    default: return -1;
    }
    case 'P':
    switch (str[1]) {
    case 'r':
    switch (str[2]) {
    case 'e':
        if (strcmp(str + 3, "ferSpecialize") == 0)
            return s_PreferSpecialize;
        break;
    case 'i':
        if (strcmp(str + 3, "mary") == 0)
            return s_Primary;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'R':
    switch (str[1]) {
    case 'e':
    switch (str[2]) {
    case 'b':
    switch (str[3]) {
    case 'i':
    switch (str[4]) {
    case 'n':
    switch (str[5]) {
    case 'd':
    switch (str[6]) {
    case 's':
        if (strcmp(str + 7, "Input") == 0)
            return s_RebindsInput;
        break;
    case 0:
            return s_Rebind;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'c':
    switch (str[3]) {
    case 'o':
        if (strcmp(str + 4, "mpileModule") == 0)
            return s_RecompileModule;
        break;
    case 'u':
        if (strcmp(str + 4, "rsiveWildcard") == 0)
            return s_RecursiveWildcard;
        break;
    default: return -1;
    }
    case 'f':
            return s_Ref;
    case 'p':
        if (strcmp(str + 3, "lace") == 0)
            return s_Replace;
        break;
    case 't':
        if (strcmp(str + 3, "urn") == 0)
            return s_Return;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'S':
    switch (str[1]) {
    case 'e':
        if (strcmp(str + 2, "tter") == 0)
            return s_Setter;
        break;
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
            return s_StackFinished;
        break;
    case 'R':
    switch (str[6]) {
    case 'e':
        if (strcmp(str + 7, "ady") == 0)
            return s_StackReady;
        break;
    case 'u':
        if (strcmp(str + 7, "nning") == 0)
            return s_StackRunning;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 't':
    switch (str[4]) {
    case 'e':
    switch (str[5]) {
    case 'm':
        if (strcmp(str + 6, "ent") == 0)
            return s_Statement;
        break;
    case 0:
            return s_State;
    default: return -1;
    }
    case 'i':
        if (strcmp(str + 5, "cErrors") == 0)
            return s_StaticErrors;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'e':
        if (strcmp(str + 3, "p") == 0)
            return s_Step;
        break;
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
    switch (str[12]) {
    case 'l':
        if (strcmp(str + 13, "ob") == 0)
            return s_StorageTypeBlob;
        break;
    case 'o':
        if (strcmp(str + 13, "ol") == 0)
            return s_StorageTypeBool;
        break;
    default: return -1;
    }
    case 'F':
        if (strcmp(str + 12, "loat") == 0)
            return s_StorageTypeFloat;
        break;
    case 'H':
    switch (str[12]) {
    case 'a':
    switch (str[13]) {
    case 'n':
        if (strcmp(str + 14, "dle") == 0)
            return s_StorageTypeHandle;
        break;
    case 's':
        if (strcmp(str + 14, "htable") == 0)
            return s_StorageTypeHashtable;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'I':
        if (strcmp(str + 12, "nt") == 0)
            return s_StorageTypeInt;
        break;
    case 'L':
        if (strcmp(str + 12, "ist") == 0)
            return s_StorageTypeList;
        break;
    case 'N':
        if (strcmp(str + 12, "ull") == 0)
            return s_StorageTypeNull;
        break;
    case 'O':
    switch (str[12]) {
    case 'b':
        if (strcmp(str + 13, "ject") == 0)
            return s_StorageTypeObject;
        break;
    case 'p':
        if (strcmp(str + 13, "aquePointer") == 0)
            return s_StorageTypeOpaquePointer;
        break;
    default: return -1;
    }
    case 'S':
    switch (str[12]) {
    case 't':
    switch (str[13]) {
    case 'a':
        if (strcmp(str + 14, "ck") == 0)
            return s_StorageTypeStack;
        break;
    case 'r':
        if (strcmp(str + 14, "ing") == 0)
            return s_StorageTypeString;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'T':
    switch (str[12]) {
    case 'e':
        if (strcmp(str + 13, "rm") == 0)
            return s_StorageTypeTerm;
        break;
    case 'y':
        if (strcmp(str + 13, "pe") == 0)
            return s_StorageTypeType;
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
            return s_StructType;
        break;
    default: return -1;
    }
    case 'y':
    switch (str[2]) {
    case 'n':
    switch (str[3]) {
    case 't':
    switch (str[4]) {
    case 'a':
    switch (str[5]) {
    case 'x':
    switch (str[6]) {
    case '_':
    switch (str[7]) {
    case 'A':
        if (strcmp(str + 8, "nonFunction") == 0)
            return s_Syntax_AnonFunction;
        break;
    case 'B':
    switch (str[8]) {
    case 'l':
        if (strcmp(str + 9, "ockStyle") == 0)
            return s_Syntax_BlockStyle;
        break;
    case 'r':
        if (strcmp(str + 9, "ackets") == 0)
            return s_Syntax_Brackets;
        break;
    default: return -1;
    }
    case 'C':
        if (strcmp(str + 8, "olorFormat") == 0)
            return s_Syntax_ColorFormat;
        break;
    case 'D':
        if (strcmp(str + 8, "eclarationStyle") == 0)
            return s_Syntax_DeclarationStyle;
        break;
    case 'E':
        if (strcmp(str + 8, "xplicitType") == 0)
            return s_Syntax_ExplicitType;
        break;
    case 'F':
        if (strcmp(str + 8, "unctionName") == 0)
            return s_Syntax_FunctionName;
        break;
    case 'I':
    switch (str[8]) {
    case 'd':
        if (strcmp(str + 9, "entifierRebind") == 0)
            return s_Syntax_IdentifierRebind;
        break;
    case 'm':
    switch (str[9]) {
    case 'p':
    switch (str[10]) {
    case 'l':
        if (strcmp(str + 11, "icitName") == 0)
            return s_Syntax_ImplicitName;
        break;
    case 'o':
        if (strcmp(str + 11, "rt") == 0)
            return s_Syntax_Import;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'n':
    switch (str[9]) {
    case 'p':
        if (strcmp(str + 10, "utFormat") == 0)
            return s_Syntax_InputFormat;
        break;
    case 't':
        if (strcmp(str + 10, "egerFormat") == 0)
            return s_Syntax_IntegerFormat;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'L':
    switch (str[8]) {
    case 'i':
    switch (str[9]) {
    case 'n':
        if (strcmp(str + 10, "eEnding") == 0)
            return s_Syntax_LineEnding;
        break;
    case 't':
        if (strcmp(str + 10, "eralList") == 0)
            return s_Syntax_LiteralList;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'M':
    switch (str[8]) {
    case 'e':
        if (strcmp(str + 9, "thodDecl") == 0)
            return s_Syntax_MethodDecl;
        break;
    case 'u':
        if (strcmp(str + 9, "ltiline") == 0)
            return s_Syntax_Multiline;
        break;
    default: return -1;
    }
    case 'N':
    switch (str[8]) {
    case 'a':
        if (strcmp(str + 9, "meBinding") == 0)
            return s_Syntax_NameBinding;
        break;
    case 'o':
    switch (str[9]) {
    case 'B':
        if (strcmp(str + 10, "rackets") == 0)
            return s_Syntax_NoBrackets;
        break;
    case 'P':
        if (strcmp(str + 10, "arens") == 0)
            return s_Syntax_NoParens;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'O':
    switch (str[8]) {
    case 'p':
        if (strcmp(str + 9, "erator") == 0)
            return s_Syntax_Operator;
        break;
    case 'r':
        if (strcmp(str + 9, "iginalFormat") == 0)
            return s_Syntax_OriginalFormat;
        break;
    default: return -1;
    }
    case 'P':
    switch (str[8]) {
    case 'a':
        if (strcmp(str + 9, "rens") == 0)
            return s_Syntax_Parens;
        break;
    case 'o':
    switch (str[9]) {
    case 's':
    switch (str[10]) {
    case 't':
    switch (str[11]) {
    case 'E':
        if (strcmp(str + 12, "qualsSpace") == 0)
            return s_Syntax_PostEqualsSpace;
        break;
    case 'F':
        if (strcmp(str + 12, "unctionWs") == 0)
            return s_Syntax_PostFunctionWs;
        break;
    case 'H':
        if (strcmp(str + 12, "eadingWs") == 0)
            return s_Syntax_PostHeadingWs;
        break;
    case 'K':
        if (strcmp(str + 12, "eywordWs") == 0)
            return s_Syntax_PostKeywordWs;
        break;
    case 'L':
        if (strcmp(str + 12, "BracketWs") == 0)
            return s_Syntax_PostLBracketWs;
        break;
    case 'N':
        if (strcmp(str + 12, "ameWs") == 0)
            return s_Syntax_PostNameWs;
        break;
    case 'O':
        if (strcmp(str + 12, "peratorWs") == 0)
            return s_Syntax_PostOperatorWs;
        break;
    case 'W':
        if (strcmp(str + 12, "s") == 0)
            return s_Syntax_PostWs;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'r':
    switch (str[9]) {
    case 'e':
    switch (str[10]) {
    case 'D':
        if (strcmp(str + 11, "otWs") == 0)
            return s_Syntax_PreDotWs;
        break;
    case 'E':
    switch (str[11]) {
    case 'n':
        if (strcmp(str + 12, "dWs") == 0)
            return s_Syntax_PreEndWs;
        break;
    case 'q':
        if (strcmp(str + 12, "ualsSpace") == 0)
            return s_Syntax_PreEqualsSpace;
        break;
    default: return -1;
    }
    case 'L':
        if (strcmp(str + 11, "BracketWs") == 0)
            return s_Syntax_PreLBracketWs;
        break;
    case 'O':
        if (strcmp(str + 11, "peratorWs") == 0)
            return s_Syntax_PreOperatorWs;
        break;
    case 'R':
        if (strcmp(str + 11, "BracketWs") == 0)
            return s_Syntax_PreRBracketWs;
        break;
    case 'W':
        if (strcmp(str + 11, "s") == 0)
            return s_Syntax_PreWs;
        break;
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 10, "perties") == 0)
            return s_Syntax_Properties;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'Q':
        if (strcmp(str + 8, "uoteType") == 0)
            return s_Syntax_QuoteType;
        break;
    case 'R':
    switch (str[8]) {
    case 'e':
    switch (str[9]) {
    case 'b':
    switch (str[10]) {
    case 'i':
    switch (str[11]) {
    case 'n':
    switch (str[12]) {
    case 'd':
    switch (str[13]) {
    case 'O':
        if (strcmp(str + 14, "perator") == 0)
            return s_Syntax_RebindOperator;
        break;
    case 'S':
        if (strcmp(str + 14, "ymbol") == 0)
            return s_Syntax_RebindSymbol;
        break;
    case 'i':
        if (strcmp(str + 14, "ngInfix") == 0)
            return s_Syntax_RebindingInfix;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'q':
        if (strcmp(str + 10, "uire") == 0)
            return s_Syntax_Require;
        break;
    case 't':
        if (strcmp(str + 10, "urnStatement") == 0)
            return s_Syntax_ReturnStatement;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'S':
        if (strcmp(str + 8, "tateKeyword") == 0)
            return s_Syntax_StateKeyword;
        break;
    case 'T':
        if (strcmp(str + 8, "ypeMagicSymbol") == 0)
            return s_Syntax_TypeMagicSymbol;
        break;
    case 'W':
    switch (str[8]) {
    case 'h':
    switch (str[9]) {
    case 'i':
    switch (str[10]) {
    case 't':
    switch (str[11]) {
    case 'e':
    switch (str[12]) {
    case 's':
    switch (str[13]) {
    case 'p':
    switch (str[14]) {
    case 'a':
    switch (str[15]) {
    case 'c':
    switch (str[16]) {
    case 'e':
    switch (str[17]) {
    case 'B':
        if (strcmp(str + 18, "eforeEnd") == 0)
            return s_Syntax_WhitespaceBeforeEnd;
        break;
    case 'P':
    switch (str[18]) {
    case 'o':
        if (strcmp(str + 19, "stColon") == 0)
            return s_Syntax_WhitespacePostColon;
        break;
    case 'r':
        if (strcmp(str + 19, "eColon") == 0)
            return s_Syntax_WhitespacePreColon;
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
    default: return -1;
    }
    default: return -1;
    }
    case 'T':
    switch (str[1]) {
    case 'a':
        if (strcmp(str + 2, "rball") == 0)
            return s_Tarball;
        break;
    case 'e':
    switch (str[2]) {
    case 'r':
    switch (str[3]) {
    case 'm':
    switch (str[4]) {
    case 'C':
        if (strcmp(str + 5, "ounter") == 0)
            return s_TermCounter;
        break;
    case 'N':
        if (strcmp(str + 5, "ame") == 0)
            return s_TermName;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 2, "oManyInputs") == 0)
            return s_TooManyInputs;
        break;
    case 'r':
        if (strcmp(str + 2, "uncate") == 0)
            return s_Truncate;
        break;
    case 'y':
    switch (str[2]) {
    case 'p':
    switch (str[3]) {
    case 'e':
    switch (str[4]) {
    case 'N':
        if (strcmp(str + 5, "ame") == 0)
            return s_TypeName;
        break;
    case 'R':
        if (strcmp(str + 5, "elease") == 0)
            return s_TypeRelease;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'U':
    switch (str[1]) {
    case 'n':
    switch (str[2]) {
    case 'c':
        if (strcmp(str + 3, "aptured") == 0)
            return s_Uncaptured;
        break;
    case 'e':
        if (strcmp(str + 3, "valuated") == 0)
            return s_Unevaluated;
        break;
    case 'i':
        if (strcmp(str + 3, "formListType") == 0)
            return s_UniformListType;
        break;
    case 'k':
        if (strcmp(str + 3, "nownIdentifier") == 0)
            return s_UnknownIdentifier;
        break;
    case 'o':
        if (strcmp(str + 3, "bservable") == 0)
            return s_Unobservable;
        break;
    case 't':
        if (strcmp(str + 3, "yped") == 0)
            return s_Untyped;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'W':
    switch (str[1]) {
    case 'a':
        if (strcmp(str + 2, "tch") == 0)
            return s_Watch;
        break;
    case 'h':
        if (strcmp(str + 2, "itespace") == 0)
            return s_Whitespace;
        break;
    case 'i':
        if (strcmp(str + 2, "ldcard") == 0)
            return s_Wildcard;
        break;
    default: return -1;
    }
    case 'a':
    switch (str[1]) {
    case 'd':
    switch (str[2]) {
    case 'd':
        if (strcmp(str + 3, "r") == 0)
            return s_addr;
        break;
    case 'v':
        if (strcmp(str + 3, "ance") == 0)
            return s_advance;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'b':
    switch (str[1]) {
    case 'l':
        if (strcmp(str + 2, "ock") == 0)
            return s_block;
        break;
    case 'r':
        if (strcmp(str + 2, "eak") == 0)
            return s_break;
        break;
    default: return -1;
    }
    case 'c':
    switch (str[1]) {
    case 'a':
        if (strcmp(str + 2, "se") == 0)
            return s_case;
        break;
    case 'o':
    switch (str[2]) {
    case 'n':
    switch (str[3]) {
    case 'd':
        if (strcmp(str + 4, "itional_done") == 0)
            return s_conditional_done;
        break;
    case 't':
        if (strcmp(str + 4, "inue") == 0)
            return s_continue;
        break;
    default: return -1;
    }
    case 'u':
        if (strcmp(str + 3, "nt") == 0)
            return s_count;
        break;
    default: return -1;
    }
    case 'u':
        if (strcmp(str + 2, "rrent_term") == 0)
            return s_current_term;
        break;
    default: return -1;
    }
    case 'd':
    switch (str[1]) {
    case 'e':
        if (strcmp(str + 2, "clared_state") == 0)
            return s_declared_state;
        break;
    case 'i':
        if (strcmp(str + 2, "scard") == 0)
            return s_discard;
        break;
    case 'o':
        if (strcmp(str + 2, "ne") == 0)
            return s_done;
        break;
    default: return -1;
    }
    case 'e':
    switch (str[1]) {
    case 'f':
        if (strcmp(str + 2, "fect") == 0)
            return s_effect;
        break;
    case 'r':
        if (strcmp(str + 2, "ror") == 0)
            return s_error;
        break;
    case 'x':
        if (strcmp(str + 2, "pr") == 0)
            return s_expr;
        break;
    default: return -1;
    }
    case 'f':
    switch (str[1]) {
    case 'a':
        if (strcmp(str + 2, "ilure") == 0)
            return s_failure;
        break;
    case 'i':
    switch (str[2]) {
    case 'l':
    switch (str[3]) {
    case 'e':
    switch (str[4]) {
    case 'n':
        if (strcmp(str + 5, "ame") == 0)
            return s_filename;
        break;
    case 0:
            return s_file;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 2, "rmat") == 0)
            return s_format;
        break;
    case 'r':
        if (strcmp(str + 2, "ames") == 0)
            return s_frames;
        break;
    default: return -1;
    }
    case 'h':
    switch (str[1]) {
    case 'a':
    switch (str[2]) {
    case 'c':
        if (strcmp(str + 3, "ks") == 0)
            return s_hacks;
        break;
    case 's':
        if (strcmp(str + 3, "_state") == 0)
            return s_has_state;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'i':
    switch (str[1]) {
    case 'd':
        if (strcmp(str + 2, "ent") == 0)
            return s_ident;
        break;
    case 'n':
    switch (str[2]) {
    case 'c':
        if (strcmp(str + 3, "oming") == 0)
            return s_incoming;
        break;
    case 'd':
        if (strcmp(str + 3, "ex") == 0)
            return s_index;
        break;
    case 'p':
        if (strcmp(str + 3, "uts") == 0)
            return s_inputs;
        break;
    case 'v':
        if (strcmp(str + 3, "alid") == 0)
            return s_invalid;
        break;
    default: return -1;
    }
    case 't':
    switch (str[2]) {
    case 'e':
    switch (str[3]) {
    case 'm':
        if (strcmp(str + 4, "s") == 0)
            return s_items;
        break;
    case 'r':
    switch (str[4]) {
    case 'a':
    switch (str[5]) {
    case 't':
    switch (str[6]) {
    case 'o':
    switch (str[7]) {
    case 'r':
    switch (str[8]) {
    case '_':
        if (strcmp(str + 9, "value") == 0)
            return s_iterator_value;
        break;
    case 0:
            return s_iterator;
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
    case 'k':
        if (strcmp(str + 1, "ey") == 0)
            return s_key;
        break;
    case 'l':
    switch (str[1]) {
    case 'a':
        if (strcmp(str + 2, "st") == 0)
            return s_last;
        break;
    case 'i':
    switch (str[2]) {
    case 's':
        if (strcmp(str + 3, "t") == 0)
            return s_list;
        break;
    case 'v':
        if (strcmp(str + 3, "eness_list") == 0)
            return s_liveness_list;
        break;
    default: return -1;
    }
    case 'o':
    switch (str[2]) {
    case 'c':
    switch (str[3]) {
    case 'a':
    switch (str[4]) {
    case 'l':
    switch (str[5]) {
    case '_':
    switch (str[6]) {
    case 's':
    switch (str[7]) {
    case 't':
    switch (str[8]) {
    case 'a':
    switch (str[9]) {
    case 't':
    switch (str[10]) {
    case 'e':
    switch (str[11]) {
    case '_':
        if (strcmp(str + 12, "key") == 0)
            return s_local_state_key;
        break;
    case 0:
            return s_local_state;
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
    case 'o':
        if (strcmp(str + 3, "p") == 0)
            return s_loop;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'm':
    switch (str[1]) {
    case 'a':
    switch (str[2]) {
    case 'd':
        if (strcmp(str + 3, "dr") == 0)
            return s_maddr;
        break;
    case 'j':
        if (strcmp(str + 3, "or_block") == 0)
            return s_major_block;
        break;
    case 'y':
        if (strcmp(str + 3, "be") == 0)
            return s_maybe;
        break;
    default: return -1;
    }
    case 'e':
    switch (str[2]) {
    case 's':
        if (strcmp(str + 3, "sage") == 0)
            return s_message;
        break;
    case 't':
        if (strcmp(str + 3, "hodCache") == 0)
            return s_methodCache;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 'n':
    switch (str[1]) {
    case 'e':
    switch (str[2]) {
    case 'w':
        if (strcmp(str + 3, "line") == 0)
            return s_newline;
        break;
    case 'x':
    switch (str[3]) {
    case 't':
    switch (str[4]) {
    case '_':
        if (strcmp(str + 5, "case") == 0)
            return s_next_case;
        break;
    case 0:
            return s_next;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
    switch (str[2]) {
    case '_':
    switch (str[3]) {
    case 'e':
        if (strcmp(str + 4, "ffect") == 0)
            return s_no_effect;
        break;
    case 's':
        if (strcmp(str + 4, "ave_state") == 0)
            return s_no_save_state;
        break;
    default: return -1;
    }
    case 'n':
        if (strcmp(str + 3, "e") == 0)
            return s_none;
        break;
    case 0:
            return s_no;
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
    switch (str[1]) {
    case 'u':
    switch (str[2]) {
    case 't':
    switch (str[3]) {
    case 'g':
        if (strcmp(str + 4, "oing") == 0)
            return s_outgoing;
        break;
    case 'p':
        if (strcmp(str + 4, "utSlot") == 0)
            return s_outputSlot;
        break;
    case 0:
            return s_out;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'p':
    switch (str[1]) {
    case 'c':
            return s_pc;
    case 'r':
        if (strcmp(str + 2, "oduceOutput") == 0)
            return s_produceOutput;
        break;
    default: return -1;
    }
    case 'r':
    switch (str[1]) {
    case 'e':
    switch (str[2]) {
    case 'p':
        if (strcmp(str + 3, "eat") == 0)
            return s_repeat;
        break;
    case 't':
        if (strcmp(str + 3, "urn") == 0)
            return s_return;
        break;
    default: return -1;
    }
    default: return -1;
    }
    case 's':
    switch (str[1]) {
    case 'e':
        if (strcmp(str + 2, "t_value") == 0)
            return s_set_value;
        break;
    case 'l':
    switch (str[2]) {
    case 'o':
    switch (str[3]) {
    case 't':
    switch (str[4]) {
    case 'C':
        if (strcmp(str + 5, "ount") == 0)
            return s_slotCount;
        break;
    case 's':
            return s_slots;
    case 0:
            return s_slot;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 't':
    switch (str[2]) {
    case 'a':
    switch (str[3]) {
    case 't':
    switch (str[4]) {
    case '_':
    switch (str[5]) {
    case 'A':
        if (strcmp(str + 6, "ppendMove") == 0)
            return stat_AppendMove;
        break;
    case 'B':
    switch (str[6]) {
    case 'y':
    switch (str[7]) {
    case 't':
    switch (str[8]) {
    case 'e':
    switch (str[9]) {
    case 'c':
    switch (str[10]) {
    case 'o':
    switch (str[11]) {
    case 'd':
    switch (str[12]) {
    case 'e':
    switch (str[13]) {
    case '_':
    switch (str[14]) {
    case 'C':
        if (strcmp(str + 15, "reateEntry") == 0)
            return stat_Bytecode_CreateEntry;
        break;
    case 'W':
        if (strcmp(str + 15, "riteTerm") == 0)
            return stat_Bytecode_WriteTerm;
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
    case 'C':
    switch (str[6]) {
    case 'a':
        if (strcmp(str + 7, "st") == 0)
            return stat_Cast;
        break;
    case 'o':
        if (strcmp(str + 7, "py") == 0)
            return stat_Copy;
        break;
    default: return -1;
    }
    case 'D':
    switch (str[6]) {
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
    switch (str[6]) {
    case 'i':
    switch (str[7]) {
    case 'n':
    switch (str[8]) {
    case 'd':
    switch (str[9]) {
    case 'E':
        if (strcmp(str + 10, "nvValue") == 0)
            return stat_FindEnvValue;
        break;
    case 'M':
        if (strcmp(str + 10, "odule") == 0)
            return stat_FindModule;
        break;
    default: return -1;
    }
    case 'i':
        if (strcmp(str + 9, "shDynamicCall") == 0)
            return stat_FinishDynamicCall;
        break;
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'G':
    switch (str[6]) {
    case 'e':
    switch (str[7]) {
    case 't':
    switch (str[8]) {
    case 'I':
    switch (str[9]) {
    case 'n':
    switch (str[10]) {
    case 'd':
    switch (str[11]) {
    case 'e':
    switch (str[12]) {
    case 'x':
    switch (str[13]) {
    case 'C':
        if (strcmp(str + 14, "opy") == 0)
            return stat_GetIndexCopy;
        break;
    case 'M':
        if (strcmp(str + 14, "ove") == 0)
            return stat_GetIndexMove;
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
    case 'H':
    switch (str[6]) {
    case 'a':
    switch (str[7]) {
    case 's':
    switch (str[8]) {
    case 'h':
    switch (str[9]) {
    case 't':
    switch (str[10]) {
    case 'a':
    switch (str[11]) {
    case 'b':
    switch (str[12]) {
    case 'l':
    switch (str[13]) {
    case 'e':
    switch (str[14]) {
    case 'D':
    switch (str[15]) {
    case 'u':
    switch (str[16]) {
    case 'p':
    switch (str[17]) {
    case 'l':
    switch (str[18]) {
    case 'i':
    switch (str[19]) {
    case 'c':
    switch (str[20]) {
    case 'a':
    switch (str[21]) {
    case 't':
    switch (str[22]) {
    case 'e':
    switch (str[23]) {
    case '_':
        if (strcmp(str + 24, "Copy") == 0)
            return stat_HashtableDuplicate_Copy;
        break;
    case 0:
            return stat_HashtableDuplicate;
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
    default: return -1;
    }
    default: return -1;
    }
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
    case 'p':
    switch (str[11]) {
    case 'r':
    switch (str[12]) {
    case 'e':
    switch (str[13]) {
    case 't':
    switch (str[14]) {
    case 'e':
    switch (str[15]) {
    case 'r':
    switch (str[16]) {
    case '_':
    switch (str[17]) {
    case 'C':
    switch (str[18]) {
    case 'o':
    switch (str[19]) {
    case 'p':
    switch (str[20]) {
    case 'y':
    switch (str[21]) {
    case 'C':
        if (strcmp(str + 22, "onst") == 0)
            return stat_Interpreter_CopyConst;
        break;
    case 'S':
        if (strcmp(str + 22, "tackValue") == 0)
            return stat_Interpreter_CopyStackValue;
        break;
    case 'T':
        if (strcmp(str + 22, "ermValue") == 0)
            return stat_Interpreter_CopyTermValue;
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
    switch (str[18]) {
    case 'y':
    switch (str[19]) {
    case 'n':
    switch (str[20]) {
    case 'a':
    switch (str[21]) {
    case 'm':
    switch (str[22]) {
    case 'i':
    switch (str[23]) {
    case 'c':
    switch (str[24]) {
    case 'F':
        if (strcmp(str + 25, "uncToClosureCall") == 0)
            return stat_Interpreter_DynamicFuncToClosureCall;
        break;
    case 'M':
    switch (str[25]) {
    case 'e':
    switch (str[26]) {
    case 't':
    switch (str[27]) {
    case 'h':
    switch (str[28]) {
    case 'o':
    switch (str[29]) {
    case 'd':
    switch (str[30]) {
    case '_':
    switch (str[31]) {
    case 'C':
        if (strcmp(str + 32, "acheHit") == 0)
            return stat_Interpreter_DynamicMethod_CacheHit;
        break;
    case 'M':
        if (strcmp(str + 32, "oduleLookup") == 0)
            return stat_Interpreter_DynamicMethod_ModuleLookup;
        break;
    case 'S':
    switch (str[32]) {
    case 'l':
    switch (str[33]) {
    case 'o':
    switch (str[34]) {
    case 'w':
    switch (str[35]) {
    case 'L':
    switch (str[36]) {
    case 'o':
    switch (str[37]) {
    case 'o':
    switch (str[38]) {
    case 'k':
    switch (str[39]) {
    case 'u':
    switch (str[40]) {
    case 'p':
    switch (str[41]) {
    case '_':
    switch (str[42]) {
    case 'H':
        if (strcmp(str + 43, "ashtable") == 0)
            return stat_Interpreter_DynamicMethod_SlowLookup_Hashtable;
        break;
    case 'M':
        if (strcmp(str + 43, "odule") == 0)
            return stat_Interpreter_DynamicMethod_SlowLookup_Module;
        break;
    default: return -1;
    }
    case 0:
            return stat_Interpreter_DynamicMethod_SlowLookup;
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
    case 'M':
        if (strcmp(str + 18, "oveStackValue") == 0)
            return stat_Interpreter_MoveStackValue;
        break;
    case 'S':
        if (strcmp(str + 18, "tep") == 0)
            return stat_Interpreter_Step;
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
    case 'L':
    switch (str[6]) {
    case 'i':
    switch (str[7]) {
    case 's':
    switch (str[8]) {
    case 't':
    switch (str[9]) {
    case 'C':
    switch (str[10]) {
    case 'a':
    switch (str[11]) {
    case 's':
    switch (str[12]) {
    case 't':
    switch (str[13]) {
    case '_':
    switch (str[14]) {
    case 'C':
        if (strcmp(str + 15, "astElement") == 0)
            return stat_ListCast_CastElement;
        break;
    case 'T':
        if (strcmp(str + 15, "ouch") == 0)
            return stat_ListCast_Touch;
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
    case 'D':
    switch (str[10]) {
    case 'u':
    switch (str[11]) {
    case 'p':
    switch (str[12]) {
    case 'l':
    switch (str[13]) {
    case 'i':
    switch (str[14]) {
    case 'c':
    switch (str[15]) {
    case 'a':
    switch (str[16]) {
    case 't':
    switch (str[17]) {
    case 'e':
    switch (str[18]) {
    case '_':
    switch (str[19]) {
    case '1':
        if (strcmp(str + 20, "00Count") == 0)
            return stat_ListDuplicate_100Count;
        break;
    case 'E':
        if (strcmp(str + 20, "lementCopy") == 0)
            return stat_ListDuplicate_ElementCopy;
        break;
    default: return -1;
    }
    case 0:
            return stat_ListDuplicate;
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
        if (strcmp(str + 7, "adFrameState") == 0)
            return stat_LoadFrameState;
        break;
    default: return -1;
    }
    case 'M':
        if (strcmp(str + 6, "ake") == 0)
            return stat_Make;
        break;
    case 'N':
    switch (str[6]) {
    case 'a':
    switch (str[7]) {
    case 'm':
    switch (str[8]) {
    case 'e':
    switch (str[9]) {
    case 'S':
    switch (str[10]) {
    case 'e':
    switch (str[11]) {
    case 'a':
    switch (str[12]) {
    case 'r':
    switch (str[13]) {
    case 'c':
    switch (str[14]) {
    case 'h':
    switch (str[15]) {
    case 'S':
        if (strcmp(str + 16, "tep") == 0)
            return stat_NameSearchStep;
        break;
    case 0:
            return stat_NameSearch;
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
    case 'W':
    switch (str[9]) {
    case 'i':
    switch (str[10]) {
    case 't':
    switch (str[11]) {
    case 'h':
    switch (str[12]) {
    case 'S':
    switch (str[13]) {
    case 'e':
    switch (str[14]) {
    case 'l':
    switch (str[15]) {
    case 'e':
    switch (str[16]) {
    case 'c':
    switch (str[17]) {
    case 't':
    switch (str[18]) {
    case 'o':
    switch (str[19]) {
    case 'r':
    switch (str[20]) {
    case '_':
    switch (str[21]) {
    case 'T':
    switch (str[22]) {
    case 'o':
    switch (str[23]) {
    case 'u':
    switch (str[24]) {
    case 'c':
    switch (str[25]) {
    case 'h':
    switch (str[26]) {
    case '_':
    switch (str[27]) {
    case 'H':
        if (strcmp(str + 28, "ashtable") == 0)
            return stat_SetWithSelector_Touch_Hashtable;
        break;
    case 'L':
        if (strcmp(str + 28, "ist") == 0)
            return stat_SetWithSelector_Touch_List;
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
    switch (str[7]) {
    case 'a':
        if (strcmp(str + 8, "ckPushFrame") == 0)
            return stat_StackPushFrame;
        break;
    case 'o':
        if (strcmp(str + 8, "reFrameState") == 0)
            return stat_StoreFrameState;
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
    case 'C':
        if (strcmp(str + 10, "reated") == 0)
            return stat_TermCreated;
        break;
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
    default: return -1;
    }
    default: return -1;
    }
    default: return -1;
    }
    case 'o':
        if (strcmp(str + 7, "uch") == 0)
            return stat_Touch;
        break;
    default: return -1;
    }
    case 'V':
        if (strcmp(str + 6, "alueCastDispatched") == 0)
            return stat_ValueCastDispatched;
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
        if (strcmp(str + 2, "ccess") == 0)
            return s_success;
        break;
    case 'w':
        if (strcmp(str + 2, "itch") == 0)
            return s_switch;
        break;
    default: return -1;
    }
    case 't':
    switch (str[1]) {
    case 'e':
        if (strcmp(str + 2, "rm") == 0)
            return s_term;
        break;
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
    switch (str[6]) {
    case 'l':
        if (strcmp(str + 7, "se") == 0)
            return tok_False;
        break;
    case 't':
        if (strcmp(str + 7, "Arrow") == 0)
            return tok_FatArrow;
        break;
    default: return -1;
    }
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
    case 'm':
        if (strcmp(str + 6, "port") == 0)
            return tok_Import;
        break;
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
        if (strcmp(str + 6, "race") == 0)
            return tok_LBrace;
        break;
    case 'P':
        if (strcmp(str + 6, "aren") == 0)
            return tok_LParen;
        break;
    case 'S':
        if (strcmp(str + 6, "quare") == 0)
            return tok_LSquare;
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
    switch (str[6]) {
    case 'f':
        if (strcmp(str + 7, "tArrow") == 0)
            return tok_LeftArrow;
        break;
    case 't':
            return tok_Let;
    default: return -1;
    }
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
        if (strcmp(str + 6, "race") == 0)
            return tok_RBrace;
        break;
    case 'P':
        if (strcmp(str + 6, "aren") == 0)
            return tok_RParen;
        break;
    case 'S':
        if (strcmp(str + 6, "quare") == 0)
            return tok_RSquare;
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
    switch (str[7]) {
    case 'i':
        if (strcmp(str + 8, "ng") == 0)
            return tok_String;
        break;
    case 'u':
        if (strcmp(str + 8, "ct") == 0)
            return tok_Struct;
        break;
    default: return -1;
    }
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
    case 'V':
        if (strcmp(str + 5, "erticalBar") == 0)
            return tok_VerticalBar;
        break;
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
    case 'p':
            return s_top;
    default: return -1;
    }
    case 'y':
        if (strcmp(str + 2, "pe") == 0)
            return s_type;
        break;
    default: return -1;
    }
    case 'u':
        if (strcmp(str + 1, "nknown") == 0)
            return s_unknown;
        break;
    case 'w':
        if (strcmp(str + 1, "atch") == 0)
            return s_watch;
        break;
    case 'y':
        if (strcmp(str + 1, "es") == 0)
            return s_yes;
        break;
    default: return -1;
    }

    return -1;
}
} // namespace circa
