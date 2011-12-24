// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct TaggedValue;

typedef int Symbol;

const Symbol InvalidSymbol = 0;

// Core symbols
const Symbol File = 1;
const Symbol Newline = 2;
const Symbol Out = 3;
const Symbol Unknown = 4;
const Symbol Repeat = 10;
const Symbol Success = 12;
const Symbol Failure = 13;

// Misc errors
const Symbol FileNotFound = 11;

// Static errors
const Symbol NotEnoughInputs = 5;
const Symbol TooManyInputs = 6;
const Symbol ExtraOutputNotFound = 7;

// VM instructions (used in Function)
const Symbol PureCall = 8;
const Symbol ControlFlowCall = 9;

const Symbol HighestBuiltinSymbol = 13;

const Symbol FirstRuntimeSymbol = 1000;

const char* symbol_get_text(Symbol symbol);
void symbol_get_text(Symbol symbol, String* string);

Symbol as_symbol(TaggedValue* tv);
void symbol_value(Symbol name, TaggedValue* tv);
void symbol_value(TaggedValue* tv, Symbol name);
void set_symbol(TaggedValue* tv, Symbol name);

Symbol register_symbol(const char* str);
Symbol string_to_symbol(const char* str);

} // namespace circa
