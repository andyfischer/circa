
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

const int HighestBuiltinSymbol = 7;

int as_symbol(TaggedValue* tv);
void symbol_value(int name, TaggedValue* tv);
void symbol_value(TaggedValue* tv, int name);

} // namespace circa
