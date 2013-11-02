// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void set_symbol_from_string(caValue* val, caValue* str);
void set_symbol_from_string(caValue* val, const char* str);

void symbol_as_string(caValue* symbol, caValue* str);

bool symbol_eq(caValue* val, Symbol s);

void symbol_initialize_global_table();
void symbol_deinitialize_global_table();
void symbol_setup_type(Type* type);

} // namespace circa
