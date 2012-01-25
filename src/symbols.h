// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct TValue;

typedef int Symbol;

const Symbol InvalidSymbol = 0;
const Symbol EmptyName = 1;

// Core symbols
const Symbol File = 2;
const Symbol Newline = 3;
const Symbol Out = 4;
const Symbol Unknown = 5;
const Symbol Repeat = 6;
const Symbol Success = 7;
const Symbol Failure = 8;

// Misc errors
const Symbol FileNotFound = 9;

// Static errors
const Symbol NotEnoughInputs = 10;
const Symbol TooManyInputs = 11;
const Symbol ExtraOutputNotFound = 12;

// VM instructions (used in Function)
const Symbol PureCall = 13;
const Symbol ControlFlowCall = 14;

// Evaluation strategies
const Symbol Default = 15;
const Symbol ByDemand = 16;

// Temporary register values
const Symbol Unevaluated = 17;
const Symbol InProgress = 18;
const Symbol Lazy = 19;

const Symbol HighestBuiltinSymbol = 19;

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
