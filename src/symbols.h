// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

Symbol symbol_from_string(const char* str);
void set_symbol_from_string(Value* val, Value* str);
void set_symbol_from_string(Value* val, const char* str);

void symbol_as_string(Value* symbol, Value* str);
const char* symbol_val_as_string(Value* symbol);
const char* symbol_as_string(Symbol symbol);

bool symbol_eq(Value* val, Symbol s);

void symbol_initialize_global_table();
void symbol_deinitialize_global_table();
void symbol_setup_type(Type* type);

} // namespace circa
