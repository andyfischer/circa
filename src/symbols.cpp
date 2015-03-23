// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "hashtable.h"
#include "list.h"
#include "names_builtin.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"

namespace circa {

Value* g_runtimeSymbolMap;    // Maps strings to Symbol values.
Value* g_runtimeSymbolTable;   // List, associating Symbol values with strings.
int g_nextRuntimeSymbol = s_LastBuiltinName + 1;

Symbol string_to_symbol(const char* str)
{
    // Find this name as an existing builtin symbol.
    int foundBuiltin = builtin_symbol_from_string(str);
    if (foundBuiltin != -1)
        return foundBuiltin;

    // Find this name as an existing runtime symbol.
    Value strVal;
    set_string(&strVal, str);

    Value* foundRuntime = hashtable_get(g_runtimeSymbolMap, &strVal);
    if (foundRuntime != NULL)
        return as_symbol(foundRuntime);

    // Create a new runtime symbol.
    Value* newRuntime = hashtable_insert(g_runtimeSymbolMap, &strVal);
    int index = g_nextRuntimeSymbol++;
    set_int(newRuntime, index);
    list_resize(g_runtimeSymbolTable, index+1);
    set_value(list_get(g_runtimeSymbolTable, index), &strVal);
    return index;
}

void set_symbol_from_string(Value* val, Value* str)
{
    set_symbol(val, string_to_symbol(as_cstring(str)));
}

void set_symbol_from_string(Value* val, const char* str)
{
    set_symbol(val, string_to_symbol(str));
}

const char* symbol_to_string(Symbol symbol)
{
    const char* builtinName = builtin_symbol_to_string(symbol);
    if (builtinName != NULL)
        return builtinName;

    Value* tableVal = list_get(g_runtimeSymbolTable, symbol);
    if (tableVal != NULL)
        return as_cstring(tableVal);

    return NULL;
}

const char* symbol_to_string(Value* symbol)
{
    return symbol_to_string(as_symbol(symbol));
}

void symbol_to_string(Value* symbol, Value* str)
{
    const char* builtinName = builtin_symbol_to_string(as_symbol(symbol));
    if (builtinName != NULL) {
        set_string(str, builtinName);
        return;
    }

    Value* tableVal = list_get(g_runtimeSymbolTable, as_symbol(symbol));
    if (tableVal != NULL) {
        ca_assert(is_string(tableVal));
        copy(tableVal, str);
        return;
    }

    set_string(str, "");
}

static void symbol_to_source_string(Value* value, Value* out)
{
    string_append(out, ":");
    string_append(out, symbol_to_string(value));
}

static int hash_func(Value* value)
{
    return as_symbol(value);
}

bool symbol_eq(Value* val, Symbol s)
{
    return is_symbol(val) && as_symbol(val) == s;
}

void symbol_initialize_global_table()
{
    g_runtimeSymbolMap = new Value();
    set_hashtable(g_runtimeSymbolMap);
    g_runtimeSymbolTable = new Value();
    set_list(g_runtimeSymbolTable, 0);
}

void symbol_deinitialize_global_table()
{
    set_null(g_runtimeSymbolMap);
    delete g_runtimeSymbolMap;
    g_runtimeSymbolMap = NULL;
    set_null(g_runtimeSymbolTable);
    delete g_runtimeSymbolTable;
    g_runtimeSymbolTable = NULL;
}

void symbol_setup_type(Type* type)
{
    reset_type(type);
    set_string(&type->name, "Symbol");
    type->storageType = s_StorageTypeInt;
    type->toString = symbol_to_source_string;
    type->hashFunc = hash_func;
}

CIRCA_EXPORT const char* circa_symbol_text(Value* symbol)
{
    return symbol_to_string(symbol);
}

CIRCA_EXPORT bool circa_symbol_equals(Value* symbol, const char* text)
{
    return strcmp(symbol_to_string(symbol), text) == 0;
}

}
