// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct TaggedValue;

// Core symbols
const int File = 1;
const int Newline = 2;
const int Out = 3;
const int Unknown = 4;
const int Repeat = 10;

// Related to static errors
const int NotEnoughInputs = 5;
const int TooManyInputs = 6;
const int ExtraOutputNotFound = 7;

// VM instructions (used in Function)
const int PureCall = 8;
const int ControlFlowCall = 9;

const int HighestBuiltinSymbol = 9;

const int FirstRuntimeSymbol = 1000;

const char* symbol_text(int symbol);
int as_symbol(TaggedValue* tv);
void symbol_value(int name, TaggedValue* tv);
void symbol_value(TaggedValue* tv, int name);
void set_symbol(TaggedValue* tv, int name);

int register_new_symbol(const char* str);

} // namespace circa
