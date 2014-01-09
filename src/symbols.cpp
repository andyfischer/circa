// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "hashtable.h"
#include "list.h"
#include "names_builtin.h"
#include "source_repro.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"

namespace circa {

Value* g_runtimeSymbolMap;    // Maps strings to Symbol values.
Value* g_runtimeSymbolTable;   // List, associating Symbol values with strings.
int g_nextRuntimeSymbol = sym_LastBuiltinName + 1;

Symbol symbol_from_string(const char* str)
{
    // Find this name as an existing builtin symbol.
    int foundBuiltin = builtin_symbol_from_string(str);
    if (foundBuiltin != -1)
        return foundBuiltin;

    // Find this name as an existing runtime symbol.
    Value strVal;
    set_string(&strVal, str);

    caValue* foundRuntime = hashtable_get(g_runtimeSymbolMap, &strVal);
    if (foundRuntime != NULL)
        return as_int(foundRuntime);

    // Create a new runtime symbol.
    caValue* newRuntime = hashtable_insert(g_runtimeSymbolMap, &strVal);
    int index = g_nextRuntimeSymbol++;
    set_int(newRuntime, index);
    list_resize(g_runtimeSymbolTable, index+1);
    set_value(list_get(g_runtimeSymbolTable, index), &strVal);
    return index;
}

void set_symbol_from_string(caValue* val, caValue* str)
{
    set_symbol(val, symbol_from_string(as_cstring(str)));
}

void set_symbol_from_string(caValue* val, const char* str)
{
    set_symbol(val, symbol_from_string(str));
}

void symbol_as_string(caValue* symbol, caValue* str)
{
    const char* builtinName = builtin_symbol_to_string(as_int(symbol));
    if (builtinName != NULL) {
        set_string(str, builtinName);
        return;
    }

    caValue* tableVal = list_get(g_runtimeSymbolTable, as_int(symbol));
    if (tableVal != NULL) {
        ca_assert(is_string(tableVal));
        copy(tableVal, str);
        return;
    }

    set_string(str, "");
}

static std::string symbol_to_source_string(caValue* value)
{
    Value str;
    symbol_as_string(value, &str);
    return std::string(":") + as_cstring(&str);
}

static void format_source(caValue* source, Term* term)
{
    std::string s = symbol_to_source_string(term_value(term));
    append_phrase(source, s.c_str(), term, tok_ColonString);
}

static int hash_func(caValue* value)
{
    return as_int(value);
}

bool symbol_eq(caValue* val, Symbol s)
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
    type->storageType = sym_StorageTypeInt;
    type->toString = symbol_to_source_string;
    type->formatSource = format_source;
    type->hashFunc = hash_func;
}

}
