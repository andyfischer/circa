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
    case name_Return: return "Return";
    case name_Continue: return "Continue";
    case name_Break: return "Break";
    case name_Discard: return "Discard";
    case name_InfixOperator: return "InfixOperator";
    case name_FunctionName: return "FunctionName";
    case name_TypeName: return "TypeName";
    case name_TermName: return "TermName";
    case name_Keyword: return "Keyword";
    case name_Whitespace: return "Whitespace";
    case name_UnknownIdentifier: return "UnknownIdentifier";
    case name_LastBuiltinName: return "LastBuiltinName";
    default: return NULL;
    }
}

} // namespace circa
