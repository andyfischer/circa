// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct TValue;

typedef int Symbol;

const Symbol InvalidSymbol = 0;
const Symbol EmptyName = 17;

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

// Evaluation strategies
const Symbol Default = 14;
const Symbol Eager = 15;

// Temporary register values
const Symbol Unevaluated = 16;
const Symbol Lazy = 18;

const Symbol HighestBuiltinSymbol = 18;

const Symbol FirstRuntimeSymbol = 1000;

const char* symbol_get_text(Symbol symbol);
void symbol_get_text(Symbol symbol, String* string);
Symbol symbol_get_namespace_first(Symbol symbol);
Symbol symbol_get_namespace_rr(Symbol symbol);

Symbol as_symbol(TValue* tv);
void symbol_value(Symbol name, TValue* tv);
void symbol_value(TValue* tv, Symbol name);
void set_symbol(TValue* tv, Symbol name);

Symbol string_to_symbol(const char* str);

} // namespace circa
