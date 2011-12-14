
#pragma once

namespace circa {

// Builtin symbols:

const int File = 1;
const int Newline = 2;
const int Out = 3;
const int Unknown = 4;

// Related to static errors
const int NotEnoughInputs = 5;
const int TooManyInputs = 6;
const int ExtraOutputNotFound = 7;

// VM instructions (used in Function)
const int PureCall = 8;
const int ControlFlowCall = 9;

const int HighestBuiltinSymbol = 9;

int as_symbol(TaggedValue* tv);
void symbol_value(int name, TaggedValue* tv);
void symbol_value(TaggedValue* tv, int name);

} // namespace circa
